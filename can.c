#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "can.h"
#include "can_buffer.h"

can_buffer_t can_rx_buffer;
can_buffer_t can_tx_buffer;
can_t can_rx_list[CAN_RX_BUFFER_SIZE];
can_t can_tx_list[CAN_TX_BUFFER_SIZE];

const prog_char _at90can_cnf[8][3] = {
  // 10 kbps
  { 0x7E,
    0x6E,
    0x7F
  },
  // 20 kbps
  { 0x62,
    0x0C,
    0x37
  },
  // 50 kbps
  { 0x26,
    0x0C,
    0x37
  },
  // 100 kbps
  { 0x12,
    0x0C,
    0x37
  },
  // 125 kbps
  { 0x0E,
    0x0C,
    0x37
  },
  // 250 kbps
  { 0x06,
    0x0C,
    0x37
  },
  // 500 kbps
  { 0x02,
    0x0C,
    0x37
  },
  // 1 Mbps
  { 0x00,
    0x0C,
    0x37
  }
};

int can_init(uint8_t bitrate) {
  //  security check
  if (bitrate >= 8)
  return 0;

  // switch CAN controller to reset mode
  CANGCON |= (1 << SWRES);

  // set CAN Bit Timing
  // (see datasheet page 260)
  CANBT1 = pgm_read_byte(&_at90can_cnf[bitrate][0]);
  CANBT2 = pgm_read_byte(&_at90can_cnf[bitrate][1]);
  CANBT3 = pgm_read_byte(&_at90can_cnf[bitrate][2]);

  // activate CAN transmit- and receive-interrupt
  CANGIT = 0;
  CANGIE = (1 << ENIT) | (1 << ENRX) | (1 << ENTX);

  // set timer prescaler to 199 which results in a timer
  // frequency of 10 kHz (at 16 MHz)
  CANTCON = 199;

  // disable all filters
  at90can_disable_filter( 0xff );

  #if CAN_RX_BUFFER_SIZE > 0
  can_buffer_init( &can_rx_buffer, CAN_RX_BUFFER_SIZE, can_rx_list );
  #endif

  #if CAN_TX_BUFFER_SIZE > 0
  can_buffer_init( &can_tx_buffer, CAN_TX_BUFFER_SIZE, can_tx_list );
  #endif

  // activate CAN controller
  CANGCON = (1 << ENASTB);

  return 1;
}

int can_send(CANMESSAGE msg)
{
  uint8_t i, mob;

  //  MOb Sender is Nr 2
  mob=2;
  //  enable MOb number mob, auto increment index, start with index = 0
  CANPAGE = (mob<<4);
  CANSTMOB &= 0;
  //  set IDE bit, length = 8
  CANCDMOB = (0<<IDE) | (msg.length);   //ide = 1: extendet, ide = 0: normal
  //  set ID
  CANIDT2 = (unsigned char) ((msg.id<<5)&0xE0);
  CANIDT1 = (unsigned char) (msg.id>>3);
  //  write data to MOb
  for (i=0; i<msg.length; i++)
  {
    CANMSG = msg.data[i];
  }
  //  wait for txbsy
  while (CANGSTA & (1<<TXBSY));
  //  send message
  CANCDMOB |= (1<<CONMOB0);
  return 1;
}

uint8_t at90can_disable_filter(uint8_t number)
{
	if (number > 14)
	{
		if (number == CAN_ALL_FILTER)
		{
			// disable interrupts
			CANIE1 = 0;
			CANIE2 = 0;

			// disable all MObs
			for (uint8_t i = 0;i < 15;i++) {
				CANPAGE = (i << 4);

				// disable MOb (read-write required)
				CANCDMOB &= 0;
				CANSTMOB &= 0;
			}

			// mark all MObs as free
			#if CAN_RX_BUFFER_SIZE == 0
			_messages_waiting = 0;
			#endif

			#if CAN_TX_BUFFER_SIZE == 0
			_free_buffer = 15;
			#endif

			return 1;
		}

		// it is only possible to serve a maximum of 15 filters
		return 0;
	}

	// set CAN Controller to standby mode
	_enter_standby_mode();

	CANPAGE = number << 4;

	// reset flags
	CANSTMOB &= 0;
	CANCDMOB = 0;

	_disable_mob_interrupt(number);

	// re-enable CAN Controller
	_leave_standby_mode();

	return 1;
}



void _disable_mob_interrupt(uint8_t mob)
{
	if (mob < 8)
		CANIE2 &= ~(1 << mob);
	else
		CANIE1 &= ~(1 << (mob - 8));
}
