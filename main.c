// coding: utf-8

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include "main.h"
#include "can.h"
#include "uart.h"

uint8_t self = 0x01;
uint8_t rec = 0xff;

CANBLOCKS_MESSAGE cbm;

void canblocks_emtpy_data(CANBLOCKS_MESSAGE *msg) {
  int i;
  msg->status = CANBLOCKSM_STATE_READY;
  msg->timer = 0;
  msg->blocklen = 0;
  msg->command = 0;
  msg->send = 0;
  msg->rec = 0;
  for(i = 0; i < CANBLOCKS_MAX; i++) {
    msg->data[i] = 0;
  }
}
int canblocks_receive(CANBLOCKS_MESSAGE *msg)
{
  // int len = 0;
  can_t getm;
  can_get_message(&getm);
  if(getm.data[1] == CANP_SYNC) {
    if(getm.data[2] == CANBLOCKS_START) {
      canblocks_emtpy_data(msg);
    }
  }
  return 0;
}

int canblocks_send(CANBLOCKS_MESSAGE *msg) {
  char *dataptr;
  size_t datasize;
  can_t sendmsg;
  int iter_send, null_in_for;
  msg->send = self;
  msg->rec = rec;
  msg->data = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  datasize = strlen(msg->data);
  msg->command = CANP_SYNC;
  if(datasize%6 != 0)
    msg->blocklen = (datasize/6)+1;
  else
    msg->blocklen = datasize/6;
  msg->timer = 0;
  msg->status = CANBLOCKSM_STATE_READY;

  sendmsg.id = msg->rec;
  sendmsg.length = 8;
  sendmsg.data[0] = msg->send;
  sendmsg.data[1] = msg->command;
  dataptr = &msg->data[0];

  null_in_for = 0;
  if(msg->command == CANP_SYNC) 
  {
    /* Send CAN_SYNC startsequence */
    sendmsg.data[2] = CANBLOCKSM_SYNC_START;
    sendmsg.data[3] = 0x00;
    sendmsg.data[4] = 0x00;
    sendmsg.data[5] = 0x00;
    sendmsg.data[6] = msg->blocklen;
    sendmsg.data[7] = CANBLOCKSM_SYNC_TYPE_STRING;

    can_send_message(&sendmsg);

    _delay_ms(10);

    while(*dataptr != '\0' && !null_in_for)
    {
      for(iter_send = 2; iter_send < 8; iter_send++) 
      {
        if(null_in_for) {
          sendmsg.data[iter_send] = 0x00;
        } else {
          sendmsg.data[iter_send] = (uint8_t)*dataptr;
        }
        dataptr++;
        if(*dataptr == '\0')
          null_in_for = 1;
      }
      can_send_message(&sendmsg);
      _delay_ms(10);
    }

    /* Send CAN_SYNC Endsequence */
    for(iter_send = 2; iter_send < 8; iter_send++) {
      sendmsg.data[iter_send] = 0x00;
    }
    can_send_message(&sendmsg);
  }
  else
  {
    int iter_send;
    for(iter_send = 2; iter_send < 8; iter_send++)
    {
      sendmsg.data[iter_send] = msg->data[iter_send - 2];
    }
    can_send_message(&sendmsg);
  }
  return 0;
}




// -----------------------------------------------------------------------------
// Main loop for receiving and sending messages.

int main(void)
{
  can_t beef;
  beef.id = 0xFF;
  beef.length = 4;
  beef.data[0] = 0xDE;
  beef.data[1] = 0xAD;
  beef.data[2] = 0xBE;
  beef.data[3] = 0xEF;
  // Initialize CAN Bus

  can_init(BITRATE_125_KBPS);
  can_set_mode(NORMAL_MODE);
  uart_init();
  _delay_ms(100);

  /*
     Everything initialized
     put welcome on UART and
     activate system interrupts
     */
  uart_putln("");
  uart_putln("----------------");
  uart_putln("cod.m CanBlocks");
  uart_putln("--test device program");
  uart_putln("  Author: Tobias Schmitt");
  uart_putln("  email: tobias.schmitt@codm.de");
  uart_putln("(c) cod.m, 2015");
  uart_putln("----------------");
  sei();


  if(can_check_free_buffer()) {
    if(can_send_message(&beef) == 0) {
      uart_putln("canmsg konnte nicht gesendet werden");
    }
  } else {
    uart_putln("kein buffer mehr frei");
  }
  canblocks_send(&cbm);
  while (1)
  {
    uart_puts("-");
    _delay_ms(10000);
  }
  return 0;
}

