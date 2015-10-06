/**
  The MIT License (MIT)

  Copyright (c) 2015, cod.m GmbH

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.


  \brief CANBLOCKS library for AT90CAN
   This is a CANBLOCKS extended adress implementation for AT90CAN mCs.
   It uses the avr-can-lib ("universelle CAN Bibliothek") by Fabian Greiff.

   https://github.com/dergraaf/avr-can-lib

   Soon I maybe will implement this directly into the lib, because I'm very
   aware that the performance is not in a good state.

  @author  Tobias Schmitt
  @email   tobias.schmitt@codm.de
  @date    25.6.2015
  */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>

#include "canblocks.h"
#include "timer.h"
#include "uart.h"
#include "can.h"

/**
  Program Code
  */

struct canblocks_frame cbframe;
uint8_t cbframe_block;
uint8_t cbframe_curr_block;
uint8_t cbframe_ready;
uint8_t cbframe_cf_counter;
can_t flowcontrol;
void cbframe_reset(void);
void cantframe_reset(can_t *src);
uint8_t wait_flowcontrol(uint32_t ms, struct flowcontrol_frame *fcf); /* wait function for flowcontrol */


void canblocks_init(void) {
  /* make DHCP Style request for CAN_ID */
  cbframe_reset();

  flowcontrol.length = 4;
  flowcontrol.flags.extended = 0;
  flowcontrol.flags.rtr = 0;
  flowcontrol.data[1] = ((CANBLOCKS_STATUS_FC << 4) | CANBLOCKS_FLOWSTAT_CLEAR);
  flowcontrol.data[2] = CANBLOCKS_BLOCKSIZE;
  flowcontrol.data[3] = CANBLOCKS_MIN_SEP_TIME;
}

int canblocks_compute_frame(can_t *frame) {
  int i, length;
  uint8_t status, sender, receiver;
  status = (frame->data[1] & 0xF0) >> 4;
  sender = frame->data[0];
  receiver = (frame->id);

  flowcontrol.id = sender;
  flowcontrol.data[0] = receiver;

  switch(status) {
    /*
    single frame00;
    */
    case CANBLOCKS_STATUS_SF:
      if(cbframe_block) {
        return CANBLOCKS_COMPRET_BUSY;
      }
      /* if not busy copy frame */
      cbframe.sender = sender;
      cbframe.rec = receiver;
      cbframe.dl = frame->data[1] & 0x0F;
      for(i = 0; i < cbframe.dl; i++) {
        cbframe.data[i] = frame->data[i+2];
      }
      /* set cbframe to ready and return complete */
      cbframe_ready = 1;
      return CANBLOCKS_COMPRET_COMPLETE;
      break;
    /*
      first frame
      of multi
    */
    case CANBLOCKS_STATUS_FF:
      if(cbframe_block) {
        return CANBLOCKS_COMPRET_BUSY;
      }
      cbframe_curr_block = 0;
      cbframe_block = 1;
      cbframe_cf_counter = 5;
      cbframe.sender = sender;
      cbframe.rec = receiver;
      cbframe.dl = ((frame->data[1] & 0x0F) << 8) | frame->data[2];
      /* uart_puts("cbframe: (sender: ");
      uart_puti(cbframe.sender, 16);
      uart_puts(") (rec: ");
      uart_puti(cbframe.rec, 16);
      uart_puts(") (dl: ");
      uart_puti(cbframe.dl, 16);
      uart_putln(")"); */
      for(i = 0; i < (frame->length - 3); i++) {
        cbframe.data[i] = frame->data[i+3];
        /*uart_puts("cbframe.data[");
        uart_puti(i, 10);
        uart_puts("] = ");
        uart_puti(cbframe.data[i], 10);
        uart_putln("");*/
      }

      if(!can_send_message(&flowcontrol)) {
        return CANBLOCKS_COMPRET_ERROR;
      }
      return CANBLOCKS_COMPRET_TRANS;
      break;
    /*
      consecutive frame
      of multibyte
    */
    case CANBLOCKS_STATUS_CF:
      /* consecutive frame */
      if(cbframe_block == 0) {
        return CANBLOCKS_COMPRET_ERROR;
      }
      /* simply copy the frame by length */
      cbframe_curr_block++;
      length = frame->length - 2;
      for(i = 0; i < length; i++) {
        cbframe.data[cbframe_cf_counter + i] = frame->data[i+2];
        /*uart_puts("cbframe.data[");
        uart_puti(cbframe_cf_counter + i, 10);
        uart_puts("] = ");
        uart_puti(cbframe.data[cbframe_cf_counter + i], 10);
        uart_putln("");*/
      }
      cbframe_cf_counter += length;
      /* look if everything's over */
      if(cbframe_cf_counter >= cbframe.dl) {
        /* frame finished */
        cbframe_ready = 1;
        return CANBLOCKS_COMPRET_COMPLETE;
      }
      if((frame->data[1] & 0x0F) == CANBLOCKS_BLOCKSIZE - 1) {
        can_send_message(&flowcontrol);
      }

      return CANBLOCKS_COMPRET_TRANS;
      break;
  }
  return CANBLOCKS_COMPRET_ERROR;
}

int canblocks_get_frame(struct canblocks_frame *dst) {
  int i;

  if(cbframe_ready) {
    /* copy cbframe to destination frame */
    dst->sender = cbframe.sender;
    dst->rec = cbframe.rec;
    dst->dl = cbframe.dl;
    for(i = 0; i < CANBLOCKS_DATA_LENGTH; i++)
      dst->data[i] = cbframe.data[i];

    /* reset cbframe state */
    cbframe_ready = 0;
    cbframe_reset();

    return 1;
  }
  return 0;
}

int canblocks_send_frame(struct canblocks_frame *frame) {
  uint8_t i, block_count;
  can_t sframe; /* sending frame */
  uint16_t send_data_index;
  struct flowcontrol_frame fcframe; /* place for a flowcontrol frame */

  cantframe_reset(&sframe);

  /*
    single frame
  */
  sframe.id = frame->rec;
  sframe.data[0] = frame->sender;
  sframe.flags.rtr = 0;
  sframe.flags.extended = 0;

  if(frame->dl <= 6) { /* if 1 frame is enough */
    sframe.length = frame->dl + 2;
    sframe.data[1] = (CANBLOCKS_STATUS_SF << 4) | frame->dl;
    for(i = 0; i < frame->dl; i++) {
      sframe.data[i + 2] = frame->data[i];
    }
    if(can_send_message(&sframe)) {
      return 1;
    } else {
      return 0;
    }
  }

  /*
    multiframe logic
  */
  /* build first frame */
  block_count = 0;
  send_data_index = 0; /*actual index where data is send */

  sframe.length = 8;
  sframe.data[1] = (CANBLOCKS_STATUS_FF << 4) | ((frame->dl & 0x0F00) >> 8);
  sframe.data[2] = (frame->dl & 0x00FF);
  for(i = 3; i < 8; i++) {
    sframe.data[i] = frame->data[send_data_index++];
  }
  if(!can_send_message(&sframe)) {
    return 0;
  }
  block_count++;
  if(!wait_flowcontrol(2000, &fcframe))
    return 0;

  timer0_timeout(fcframe.septime);
  while(!timer0_timeout(fcframe.septime));


  /* while still bytes to send */
  while((frame->dl - send_data_index) > 0) { /* if there are still bytes left */
    if((frame->dl - send_data_index) > 6) { /* if it's more than 6 bytes (not last msg) */
      /* build consecutive frame */
      sframe.length = 8;
      sframe.data[1] = (CANBLOCKS_STATUS_CF << 4) | block_count;
      for(i = 2; i < 8; i++) {
        sframe.data[i] = frame->data[send_data_index++];
      }

      /* send frame */
      if(!can_send_message(&sframe)) {
        return 0;
      }
      block_count++;

      /* check if has to wait for flowcontrol */
      if(block_count > CANBLOCKS_BLOCKSIZE - 1) {
        block_count = 0;
        if(!wait_flowcontrol(2000, &fcframe))
            return 0;
      }
      timer0_timeout(fcframe.septime);
      while(!timer0_timeout(fcframe.septime));

    } else {
      /* compute last length and build frame, send */
      sframe.length = (frame->dl - send_data_index) + 2;
      sframe.data[1] = (CANBLOCKS_STATUS_CF << 4) | block_count;
      for(i = 2; i < sframe.length; i++) {
        sframe.data[i] = frame->data[send_data_index++];
      }
      /* send frame */
      if(!can_send_message(&sframe)) {
        return 0;
      }
    }
  }
  return 1;
}


int canblocks_fr2str(char *dst, struct canblocks_frame *src) {
    char *buffer = dst;
    int i, n;
    n = sprintf(buffer, "%02x;", src->sender);
    buffer = buffer+n;
    n = sprintf(buffer, "%02x;", src->rec);
    buffer = buffer+n;
    n = sprintf(buffer, "%04u;", src->dl);
    buffer = buffer+n;
    for(i = 0; i < src->dl; i++) {
        n = sprintf(buffer, "%02x", src->data[i]);
        buffer = buffer+n;
    }
    *buffer = '\n';
    return 1;
}

int canblocks_str2fr(char *src, struct canblocks_frame *dst) {
    unsigned int i, sender, rec, dl;
    uint8_t *bufdst;
    char buffer[2*4096]; /* 4096 uint8_t a 2 characters */
    char *bufbuff = buffer;
    /* TODO: Secure this input via regex */
    if(sscanf(src, "%02x;%02x;%04u;%s", &sender, &rec, &dl, buffer) < 1) {
        return 0;
    };
    dst->sender = (uint8_t)sender;
    dst->rec = (uint8_t)rec;
    dst->dl = (uint8_t)dl;
    bufdst = dst->data;
    for(i = 0; i < dst->dl; i++) {
        sscanf(bufbuff, "%02x", &rec); /* read 2 chars put into byte */
        *bufdst = (uint8_t)rec;
        bufdst++; /* iterate over array */
        bufbuff = bufbuff + 2; /* iterate over 2 chars in string */
    }
    return 1;
}

void cbframe_reset() {
  int i;
  cbframe.sender = 0x00;
  cbframe.rec = 0x00;
  cbframe.dl = 0;
  for(i = 0; i < CANBLOCKS_DATA_LENGTH; i++)
    cbframe.data[i] = 0x00;
  cbframe_ready = 0;
  cbframe_cf_counter = 0;
  cbframe_curr_block = 0;
  cbframe_block = 0;
}

void cantframe_reset(can_t *src) {
    int i;
    src->id = 0;
    src->length = 0;
    src->flags.rtr = 0;
    src->flags.extended = 0;
    for(i = 0; i < 8; i++) {
        src->data[i] = 0;
    }
}

void canblocks_reset_frame(struct canblocks_frame *src) {
    int i;
    src->sender = 0x00;
    src->rec = 0x00;
    src->dl = 0;
    for(i = 0; i < CANBLOCKS_DATA_LENGTH; i++)
        src->data[i] = 0x00;
}

uint8_t wait_flowcontrol(uint32_t ms, struct flowcontrol_frame *fcf) {
    can_t getmsg;
    uint8_t status;
    timer0_timeout(ms);
    while(!timer0_timeout(0)) {
        if(can_check_message() && can_get_message(&getmsg)) {
            status = (getmsg.data[1] & 0xF0) >> 4;
            if(status == CANBLOCKS_STATUS_FC) {
                fcf->sender = getmsg.data[0];
                fcf->rec = getmsg.id;
                fcf->flowstatus = getmsg.data[1] & 0x0F;
                fcf->blocksize = getmsg.data[2];
                fcf->septime = getmsg.data[3];
                return 1;
            }
        }
    }
    return 0;
}
