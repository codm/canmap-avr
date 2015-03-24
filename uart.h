/*
 * uart.h
 *
 *  Created on: Jan 4, 2014
 *      Author: codm
 */
#ifndef UART_H_
#define UART_H_

// Includes
#include <avr/io.h>
#include <stdlib.h>
#include "main.h"


// Berechnungen
#ifndef F_CPU
#define F_CPU 16000000UL
#warning uart.h: F_CPU was not defined. Default set to 1MHz
#endif

#ifndef BAUD
#define BAUD 9600
#warning uart.h: BAUD was not defined. Default set to 9600
#endif

#define UBRR_VAL ((F_CPU+BAUD*8)/(BAUD*16)-1)
#define BAUD_REAL (F_CPU/(16*(UBRR_VAL+1)))
#define BAUD_ERROR ((BAUD_REAL*1000)/BAUD)

#if ((BAUD_ERROR<990) || (BAUD_ERROR>1010))
#error Systematischer Fehler der Baudrate grösser 1% und damit zu hoch!
#endif

/* UART-Init Bsp. ATmega48 */

extern void uart_init(void);
/* ATmega16 */
extern int uart_putc(unsigned char c);

/* puts ist unabhaengig vom Controllertyp */
extern void uart_puts(char *s);

/* puts UART line with \r\n */
extern void uart_putln(char *s);

/* puts digits as ascii */
extern void uart_putl(unsigned long val, int base);
extern void uart_puti(unsigned int val, int base);

/* timestamp functions */
extern void uart_puttimestamp(long rt);

#endif /* UART_H_ */
