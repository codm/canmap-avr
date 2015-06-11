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

canblocksmsg_t cb_rec;
canblocksmsg_t cb_send;


int can_printdebug(char prefix, canblocksmsg_t *msg) {
    int i;
    if(msg->type == CANBLOCKSM_TYPE_NORMAL) {
        uart_putc(prefix);
        uart_puts(" single - snd: ");
        uart_puti(msg->send, 16);
        uart_puts(" rec: ");
        uart_puti(msg->rec, 16);
        uart_puts(" cmd: ");
        uart_puti(msg->command, 16);
        uart_puts(" [");
        for(i = 0; i < 6; i++) {
            uart_puti(msg->singledata[i], 16);
            uart_putc(';');
        }
        uart_putln("]");
    } else if(msg->type == CANBLOCKSM_TYPE_STRING) {
        uart_putc(prefix);
        uart_puts(" block - snd: ");
        uart_puti(msg->send, 16);
        uart_puts(" rec: ");
        uart_puti(msg->rec, 16);
        uart_puts(" msg: ");
        uart_puts(&msg->blockdata[0]);
        uart_putln("");
    }
    return 1;
}


// -----------------------------------------------------------------------------
// Main loop for receiving and sending messages.

int main(void)
{
    /*
    can_t beef;
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
    */
    /* init cbsend */
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
    /* can_send_message(&beef); */

    while (1)
    {
        if(canblocks_receive(&cb_rec) == 1) {
            /* print */
            int b;
            can_printdebug('+', &cb_rec);
            b = cb_rec.send;
            cb_rec.send = cb_rec.rec;
            cb_rec.rec = b;
            canblocks_send(&cb_rec);
            can_printdebug('-', &cb_rec);
            canblocks_reset_data(&cb_rec);
        }
    }
    return 0;
}

