/*
 * uart.c
 *
 *  Created on: Jan 6, 2014
 *      Author: codm
 */

#include "uart.h"


/* UART-Init Bsp. ATmega48 */

void uart_init(void) {
	UBRR0H = UBRR_VAL >> 8;
	UBRR0L = UBRR_VAL & 0xFF;

	UCSR0B |= (1 << TXEN0);  // UART TX einschalten
	UCSR0C = /*(1 << UMSEL0) |*/ (1 << UCSZ1) | (1 << UCSZ0);  // Asynchron 8N1
}

/* ATmega16 */
int uart_putc(unsigned char c) {
	while (!(UCSR0A & (1 << UDRE0))) /* warten bis Senden moeglich */
	{
	}

	UDR0 = c; /* sende Zeichen */
	return 0;
}

/* puts ist unabhaengig vom Controllertyp */
void uart_puts(char *s) {
	while (*s) { /* so lange *s != '\0' also ungleich dem "String-Endezeichen(Terminator)" */
		uart_putc(*s);
		s++;
	}
}

/* puts UART line with \r\n */
void uart_putln(char *s)
{
	while(*s)
	{
		uart_putc(*s);
		s++;
	}
	uart_putc('\r');
	uart_putc('\n');
}

void uart_puti(unsigned int val, int base) {
    char prbuff[32];
    itoa(val, &prbuff[0], base);
    uart_puts(&prbuff[0]);
}

void uart_putl(unsigned long val, int base) {
    char pfbuff[32];
    ltoa(val, &pfbuff[0], base);
    uart_puts(&pfbuff[0]);
}

void uart_puttimestamp(long rt) {
    int d = (((rt/1000)/60)/60)/24;
    int h = (((rt/1000)/60)/60) - (d*24);
    long m = ((rt/1000)/60) - (h*60) - (24*d);
    long s = (rt/1000) - (60*m);
    uart_putc('<');
    if(d != 0) {
        uart_puti(d, 10);
        uart_putc('d');
    }
    if(h != 0) {
        uart_puti(h, 10);
        uart_putc('h');
    }
    if(m != 0) {
        uart_puti(m, 10);
        uart_putc('m');
    }
    uart_putl(s, 10);
    uart_puts("s>\t");
}

