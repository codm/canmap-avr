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
#include "timer.h"

uint8_t self = 0x01;
uint8_t rec = 0xff;

char uart_buff[256];
void print_blockframe(struct canblocks_frame *src);


// -----------------------------------------------------------------------------
// Main loop for receiving and sending messages.

int main(void)
{
    /* init vars */
    can_t getmsg;
    struct canblocks_frame getframe;
    
    can_t smsg = {
        .id = 0xFF,
        .length = 4,
        .flags.rtr = 0,
        .flags.extended = 0,
        .data = {0xDE, 0xAD, 0xBE, 0xEF}
    };

    can_filter_t filter = {
        .id = 0,
        .mask = 0,
        .flags = {
            .rtr = 0,
            .extended = 0,
        }
    };

    struct canblocks_frame sframe = {
        .sender = 0x01,
        .rec = 0x00,
        .dl = 72,
        .data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x01,
                0x01, 0x02, 0x03, 0x04, 0x05, 0x01,
                0x01, 0x02, 0x03, 0x04, 0x05, 0x01,
                0x01, 0x02, 0x03, 0x04, 0x05, 0x01,
                0x01, 0x02, 0x03, 0x04, 0x05, 0x01,
                0x01, 0x02, 0x03, 0x04, 0x05, 0x01,
                0x01, 0x02, 0x03, 0x04, 0x05, 0x01,
                0x01, 0x02, 0x03, 0x04, 0x05, 0x01,
                0x01, 0x02, 0x03, 0x04, 0x05, 0x01,
                0x01, 0x02, 0x03, 0x04, 0x05, 0x01,
                0x01, 0x02, 0x03, 0x04, 0x05, 0x01,
                0x01, 0x02, 0x03, 0x04, 0x05, 0x01}
    };

    /* struct canblocks_frame cblocks; */

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

    sei();
    can_set_filter(0, &filter);

    uart_putln("CAN init success");
    uart_putln("----------------");

    timer0_init();
    uart_putln("timer0_init");
    /* uart_putln("test 2 sec");
    timer0_timeout(2000);
    uint32_t stamp = timer0_get_ms_stamp();
    uart_puti(stamp, 10);
    uart_puts("ms -> ");
    while(!(timer0_timeout(0)));
    stamp = timer0_get_ms_stamp();
    uart_puti(stamp, 10);
    uart_putln("ms"); */ 
    uart_putln("----------------");
    canblocks_init();
    can_send_message(&smsg);
    while (1) {
        int ret;
        if(can_check_message() && can_get_message(&getmsg)) {
            ret = canblocks_compute_frame(&getmsg);
            switch(ret) {
                case CANBLOCKS_COMPRET_COMPLETE:
                    canblocks_get_frame(&getframe);
                    print_blockframe(&getframe);
                    uart_putln("sending frame...");
                    if(canblocks_send_frame(&sframe)) {
                        uart_putln("success");
                        print_blockframe(&sframe);
                    } else {
                        uart_putln("could not send canblocksframe....");
                    }
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
    uart_puts(" + [");
    uart_puti(src->dl, 10);
    uart_puts("] ");
    uart_puti(src->sender, 16);
    uart_puts("->");
    uart_puti(src->rec, 16);
    uart_putln(": <TODO: DATAPRINT>");
}

void print_timestamp(uint8_t linebreak) {
    uint8_t sec,min;
    uint16_t h;
    timer0_gettime(&sec, &min, &h);
    uart_putc('<');
    uart_puti(h, 10);
    uart_putc(':');
    uart_puti(min, 10);
    uart_putc(':');
    uart_puti(sec, 10);
    uart_putc('>');
    if(linebreak)
        uart_putln("");
}
