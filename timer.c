/**
* timer.c - AT90CAN128 Timer Settings
* author: Tobias Schmitt
* date: 2015-09-30
* company: cod.m GmbH
*/
#include <avr/io.h>
#include <avr/interrupt.h>

#include "uart.h"

uint32_t __time_ms;
uint8_t __time_sec;
uint8_t __time_min;
uint16_t __time_h;

uint32_t __timer0_timeout;

void timer0_init(void) {
    __timer0_timeout = 0;
    __time_ms = 0;
    __time_sec = 0;
    __time_min = 0;
    __time_h = 0;

    TCCR0A = 0b00001011; /* WGM01 / CS01 / CS00 ON */
    /* TCCR0A = (1<<WGM01) & (1<<CS01) & (1 << CS00); settings: Clear on Compare on, run in clock / 8 */
    /* 16.000.000 Hz
    /       64 Prescaler
    = 250.000 Hz
    /     1.000
    =       250 Counts per ms*/
    OCR0A = 250 - 1;
    TIMSK0 = (1<<OCIE0A);
}

uint32_t timer0_gettime(uint8_t *sec, uint8_t *min, uint16_t *h) {
    *sec = __time_sec;
    *min = __time_min;
    *h = __time_h;
    return __time_ms;
}

uint32_t timer0_get_ms_stamp(void) {
    return __time_ms;
}

uint8_t timer0_timeout(uint32_t ms) {
    if(ms > 0) {
        __timer0_timeout = __time_ms + ms;
        return 1;
    } else {
        if(__time_ms > __timer0_timeout) {
            __timer0_timeout = 0;
            return 1; /* timeout finished */
        }
        return 0; /* timeout still running */
    }
}

/**
* Interrupt alle 1ms
*/
ISR (TIMER0_COMP_vect) {
    __time_ms++;
    if(__time_ms % 1000 == 0) {
        __time_sec++;
        if(__time_sec % 60 == 0) {
            __time_sec = 0;
            __time_min++;
            if(__time_min % 60 == 0)
            __time_min = 0;
            __time_h++;
        }
    }
}
