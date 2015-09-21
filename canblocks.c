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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <linux/can.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <string.h>

#include "canblocks.h"
#include "main.h"

/**
  canblocks-Buffer
  this is not a ringbuffer, because the creation of one buffer element and
  it's completion / deletion depends on more than 1 CAN messages, which will
  be send asynchronously. so the buffer functions have to "search" the whole
  array everytime.
  */

/**
  Program Code
  */

void canblocks_init(void) {
  
}

int canblocks_compute_frame(int *socket, struct can_frame *frame) {
}

int canblocks_get_frame(struct canblocks_frame *dst) {
}

int canblocks_send_frame(struct canblocks_frame *frame) {

    /* zero rfds and 10000 usec wait */
    /* single frame */
    /* build first frame */
    /* set sock option for timeout */
    /* wait for FC with timeout */

    /* while still bytes to send */
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
    dst->data = malloc(sizeof(uint8_t) * dst->dl);
    bufdst = dst->data;
    for(i = 0; i < dst->dl; i++) {
        sscanf(bufbuff, "%02x", &rec); /* read 2 chars put into byte */
        *bufdst = (uint8_t)rec;
        bufdst++; /* iterate over array */
        bufbuff = bufbuff + 2; /* iterate over 2 chars in string */
    }
    return 1;
}

void canblocks_reset_frame(struct canblocks_frame *dst) {
    free(dst->data);
    dst->sender = 0;
    dst->rec = 0;
    dst->dl = 0;
}

