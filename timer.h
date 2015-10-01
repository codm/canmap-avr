/**
  * timer.h - AT90CAN128 Timer Settings
  * author: Tobias Schmitt
  * date: 2015-09-30
  * company: cod.m GmbH
  */

void timer0_init(void);

uint32_t timer0_gettime(uint8_t *sec, uint8_t *min, uint32_t *h);

uint32_t timer0_get_ms_stamp(void);

uint8_t timer0_timeout(uint32_t ms);
