/* STDLIB includes */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/* Project Includes */
#include "main.h"
#include "globals.h"
#include "uart.h"
#include "can.h"
#include "canblocks.h"

uint8_t self = 0x01;
uint8_t rec = 0xff;

char uart_buff[256];
void print_blockframe(struct canblocks_frame *src);


// -----------------------------------------------------------------------------
// Main loop for receiving and sending messages.

int main(void)
{
    can_t smsg;
    can_t getmsg;
    struct canblocks_frame blockframe;
    /* struct canblocks_frame cblocks; */

    smsg.id = 0x00;
    smsg.length = 2;
    smsg.flags.rtr = 0;
    smsg.flags.extended = 0;
    smsg.data[0] = 0x01;
    smsg.data[1] = 0x02;

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
    can_init(BITRATE_125_KBPS);
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

    uart_putln("CAN init success");
    uart_putln("----------------");
    canblocks_init();
    can_send_message(&smsg);
    while (1)
    {
        int ret;
        if(can_check_message() && can_get_message(&getmsg)) {
            ret = canblocks_compute_frame(&getmsg);
            switch(ret) {
                case CANBLOCKS_COMPRET_COMPLETE:
                    canblocks_get_frame(&blockframe);
                    print_blockframe(&blockframe);
                    break;
                case CANBLOCKS_COMPRET_TRANS:
                    /*uart_putln("trans pending...");*/
                    break;
                case CANBLOCKS_COMPRET_ERROR:
                    uart_putln("trans error...");
                    break;
                case CANBLOCKS_COMPRET_BUSY:
                    uart_putln("trans engine busy...");
                    break;
                default:
                    uart_putln("somethings very wrong...");
            }
        }   
    }
    return 0;
}

void print_blockframe(struct canblocks_frame *src) {
    uart_puts("+[");
    uart_puti(src->dl, 10);
    uart_puts("] ");
    uart_puti(src->sender, 16);
    uart_puts("->");
    uart_puti(src->rec, 16);
    uart_putln(": <TODO: DATAPRINT>");
}
