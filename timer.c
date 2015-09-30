/**
  * timer.c - AT90CAN128 Timer Settings
  * author: Tobias Schmitt
  * date: 2015-09-30
  * company: cod.m GmbH
  */

uint32_t __time_ms;
uint8_t __time_sec;
uint8_t __time_min;
uint16_t __time_h;

void timer0_init(void) {
	TCCR0A = (1<<WGM01) & (1<<CS01) & (1 << CS00); /* settings: Clear on Compare on, run in clock / 8 */
	TIMSK0 = (1<<OCIE0A);
	/* 16.000.000 Hz
		/	   64 Prescaler
		= 250.000 Hz
		/ 	1.000
		= 	  250 Counts per ms*/
	OCR0A = 250;
}

