

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
   @date    24.6.2015
*/

#ifndef _CANBLOCKS_H
#define _CANBLOCKS_H

/*
   Defines
*/
#include "can.h"

#define CANBLOCKS_BUFFER_SIZE        20      /* Buffer size can be chosen freely */
#define CANBLOCKS_DATA_LENGTH        256
#define CANBLOCKS_BLOCKSIZE          4       /* Maximum 16 Blocks  */
#define CANBLOCKS_MIN_SEP_TIME       25      /* Min 10ms Seperation time  */
#define CANBLOCKS_BROADCAST          0xFF    /* Broadcast Adress */

#define CANBLOCKS_STATUS_SF          0x00    /* Single Frame */
#define CANBLOCKS_STATUS_FF          0x01    /* First Frame */
#define CANBLOCKS_STATUS_CF          0x02    /* Consecutive Frames */
#define CANBLOCKS_STATUS_FC          0x03    /* Flow Control Frame */

#define CANBLOCKS_COMPRET_COMPLETE   1       /* Transmission Complete */
#define CANBLOCKS_COMPRET_TRANS      0       /* Transmission pending... */
#define CANBLOCKS_COMPRET_ERROR      -1      /* No ISO-TP Frame or no fre buffer */
#define CANBLOCKS_COMPRET_BUSY       2       /* CANBLOCKS Mechanism Busy with another message */

#define CANBLOCKS_FLOWSTAT_CLEAR     0
#define CANBLOCKS_FLOWSTAT_WAIT      1
#define CANBLOCKS_FLOWSTAT_OVERFLOW  2

/**
  \brief Abstract struct of a ISO-TP frame
  this is a typical ISO-TP frame for extended CAN Addressing
*/
struct canblocks_frame {
    uint8_t sender; /**< Sender-ID of ISO-TP Frame */
    uint8_t rec; /**< Receiver-ID of ISO-TP Frame */
    uint16_t dl; /**< Length of ISO-TP Frame */
    uint8_t data[CANBLOCKS_DATA_LENGTH]; /**< Data Pointer of ISO-TP Frame */
};

/**
  \brief !MANDATORY! init the needed canblocks data structs
*/


void canblocks_init(void);
/**
  \brief computes can_frame into internal buffer
  This function computes a can_frame into its internal iso_tp
  framebuffer. If this was the last frame of an ISO-TP message,
  the funtions returns status > 0. If there are still messages missing
  the function returns 0 and -1 if there was an error executing

  @param[0] int *socket        pointer to an open CAN_SOCKET
  @param[0] can_frame *frame can frame to be processed

  @return < 0 for error, 0 if there are still messages to come
            1 if the canblocks_frame is finished and ready to get
*/
int canblocks_compute_frame(can_t *frame);

/**
  \brief sends an canblocks frame over socket

  @param[0] int *socket        pointer to an open CAN_SOCKET
  @param[0] canblocks_frame *frame frame to be sent

  @return EXIT_SUCCESS for success
          EXIT_FAILURE for failure

*/
int canblocks_send_frame(struct canblocks_frame *frame);

/**
  \brief gets a finished ISO-TP frame for further computation

  @param[0] canblocks_frame **dst pointer to a frame which should be written

  @return EXIT_SUCCESS for success
          EXIT_FAILURE for failure
*/
int canblocks_get_frame(struct canblocks_frame *dst);
void canblocks_reset_frame(struct canblocks_frame *dst);

int canblocks_fr2str(char *dst, struct canblocks_frame *src);
int canblocks_str2fr(char *src, struct canblocks_frame *dst);
#endif
  
