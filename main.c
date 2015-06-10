// coding: utf-8

/* STDLIB includes */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>

/* Project Includes */
#include "main.h"
#include "globals.h"
#include "uart.h"
#include "can.h"
#include "canblocks.h"

uint8_t self = 0x01;
uint8_t rec = 0xff;

char uart_buff[256];



int can_printdebug(char *prefix, can_t *message) {
    /*char buff[96];
      sprintf(buff, "%s [%04X] - %2d (%02X %02X %02X %02X %02X %02X %02X %02X)",
      prefix, (unsigned int)message->id, message->length, message->data[0],
      message->data[1], message->data[2], message->data[3], message->data[4],
      message->data[5], message->data[6], message->data[7]);
      uart_putln(&buff[0]);*/
    uart_puti(message->data[1], 16);
    return 1;
}


// -----------------------------------------------------------------------------
// Main loop for receiving and sending messages.

int main(void)
{
    can_t beef/*, getmsg*/;
    beef.id = 0x01;
    beef.length = 8;
    beef.data[0] = 0xDE;
    beef.data[1] = 0xAD;
    beef.data[2] = 0xBE;
    beef.data[3] = 0xEF;
    beef.data[4] = 0xDE;
    beef.data[5] = 0xAD;
    beef.data[6] = 0xBE;
    beef.data[7] = 0xEF;

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
    uart_putln("  test device program");
    uart_putln("  Author: Tobias Schmitt");
    uart_putln("  email: tobias.schmitt@codm.de");
    uart_putln("(c) cod.m, 2015");
    uart_putln("----------------");
    can_init(4);
    can_filter_t filter = {
        .id = 0,
        .mask = 0,
        .flags = {
            .rtr = 0,
            .extended = 0,
        }
    };
    sei();

    can_set_filter(0, &filter);
    can_send_message(&beef);

    canblocks_send(&canblocks_message);
    while (1)
    {
        if (can_check_message())
        {
            can_t msg;

            // Try to read the message
            if (can_get_message(&msg))
            {
                // If we received a message resend it with a different id

                // Send the new message
                //can_send_message(&msg);
                uart_puts("+: ");
                uart_puti(msg.data[1], 10);
                uart_putln("");
            }
        }
    }
    return 0;
}
