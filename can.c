#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "can.h"

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
  CAN_PORT |= _BV(CAN_TX);
  CAN_PORT &= ~_BV(CAN_RX);

  //  security check
  if (bitrate >= 8) return 0;

  //  number of the MOb
  uint8_t mob;
  // set rx and tx pins
  CAN_PORT &= ~(1 << CAN_RX);
  CAN_PORT |= (1 << CAN_TX);

  //Reset CAN Controller
  CANGCON |= (1 << SWRES);
  CANGCON = 0x00;


  // reset all registers
  CANSIT2 = 0X00;
  CANSIT1 = 0X00;
  CANGIT = 0x00;
  CANGIE = 0x00;
  CANEN1 = 0x00;
  CANEN2 = 0x00;
  CANIE1 = 0x00;
  CANIE2 = 0x00;

  // set autoincrement
  CANPAGE &= ~(1 << AINC);

  // reset all mob
  for (mob = 0; mob < NR_MOBS; mob++)
  {
    CANPAGE  = (mob << 4);
    CANIDT1 = 0x00;  //  reset ID-Tag
    CANIDT2 = 0x00;
    CANIDT3 = 0x00;
    CANIDT4 = 0x00;

    CANIDM1 = 0x00;  //  reset ID-Mask
    CANIDM2 = 0x00;
    CANIDM3 = 0x00;
    CANIDM4 = 0x00;

    CANSTMOB = 0x00;  //  reset MOb status
    CANCDMOB = 0x00;  //  disable MOb
  }
  return 0;


  // set CAN Bit Timing,(see datasheet page 260)
  CANBT1 = pgm_read_byte(&_at90can_cnf[bitrate][0]);
  CANBT2 = pgm_read_byte(&_at90can_cnf[bitrate][1]);
  CANBT3 = pgm_read_byte(&_at90can_cnf[bitrate][2]);

  // set config to MObs 1 and 2
  // MOb 1 soll empfangen
  // MOb 2 soll senden
  for (mob = 1; mob < 3; mob++)
  {
    CANPAGE  = (mob << 4);
    CANSTMOB = 0x00;  //  reset MOb status
    switch (mob)
    {
      case 1:
        CANCDMOB = 0x80;  //  RX
        CANIDT1  = 0x00;  //  set ID-Tag
        CANIDT2  = 0x00;

        CANIDM1  = 0x00;  //  set ID-Mask, receive all
        CANIDM2  = 0x00;
        break;

      case 2:
        CANIDT1  = 0x00;  //  set ID-Tag
        CANIDT2  = 0x00;
        break;

      default:
        return 0;
    }
  }

  //  Enable all required interrupts
  CANGIE  = 0xB0;  // ENIT, ENRX, ENTX
  CANIE2  = 0x06;  // MOb 1, MOb 2 aktivieren

  //  switch CAN on
  CANGCON |= (1<<ENASTB);

  //  wait for EnableFlag
  while (!(CANGSTA & (1<<ENFG)));

  return 1;
}

int can_send(CANMESSAGE msg)
{
  uint8_t i, mob;

  //  MOb Sender is Nr 2
  mob=2;
  //  enable MOb number mob, auto increment index, start with index = 0
  CANPAGE = (mob<<4);
  //  set IDE bit, length = 8
  CANCDMOB = (0<<IDE) | (msg.length);   //ide = 1: extendet, ide = 0: normal
  //  set ID
  CANIDT2 = (unsigned char) ((msg.id<<5)&0xE0);
  CANIDT1 = (unsigned char) (msg.id>>3);
  //  write data to MOb
  for (i=0; i<8; i++)
  {
    CANMSG = msg.data[i];
  }
  //  wait for txbsy
  while (CANGSTA & (1<<TXBSY));
  //  send message
  CANCDMOB |= (1<<CONMOB0);
  return 1;
}
