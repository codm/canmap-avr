#ifndef CAN_H_
#define CAN_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include <avr/io.h>

// -----------------------------------------------------------------------------
// pin and register definitions
// -----------------------------------------------------------------------------


#define CAN_PORT DDRD
#define CAN_RX PD6
#define CAN_TX PD7
#define CAN_RX_BUFFER_SIZE 16
#define CAN_TX_BUFFER_SIZE 8

// -----------------------------------------------------------------------------
// can bus general definitions
// -----------------------------------------------------------------------------

#define NR_MOBS 15
#define CANP_REQUEST(a) ( a < 0x42 );
#define CANP_REQANSWER 0x43
#define CANP_BLINK 0x44
#define CANP_RESET 0x45
#define CANP_SYNC 0xFA
#define CANP_ACK 0xFB

#define CANBLOCKS_MAX 255
#define CANBLOCKS_START 0xFF
#define CANBLOCKS_END 0xFE
#define CANBLOCKS_DELAY 0x0A

#define CANBLOCKST_STRING 0x01

#define CANBLOCKSM_STATE_READY 0x00
#define CANBLOCKSM_STATE_TRANS 0x01
#define CANBLOCKSM_STATE_FIN   0x02

#define CANBLOCKSM_SYNC_START 0x01

#define CANBLOCKSM_SYNC_TYPE_STRING 0x01

#define CAN_SELF 0x01
#define CAN_OTHER 0x02

#define CAN_ALL_FILTER 0xff


// -----------------------------------------------------------------------------
// typedefinitions
// -----------------------------------------------------------------------------

typedef uint8_t prog_char;

typedef struct {
    uint16_t id;
    uint16_t mask;
    uint8_t length;
    uint8_t data[8];
} CANMESSAGE;

typedef CANMESSAGE can_t;

typedef struct {
    uint8_t rec;
    uint8_t send;
    uint8_t command;
    uint8_t blocklen;
    uint32_t timer;
    char* data;
    uint8_t status; // 0: waiting, 1: transmittions, 2: ready
} CANBLOCKS_MESSAGE;



extern int can_init(uint8_t bitrate);
extern int can_send(CANMESSAGE msg);
extern uint8_t at90can_disable_filter(uint8_t number);
extern void _disable_mob_interrupt(uint8_t mob);


extern __attribute__ ((gnu_inline)) inline void _enter_standby_mode(void)
{
	// request abort
	CANGCON = (1 << ABRQ);
	
	// wait until receiver is not busy
	while (CANGSTA & (1 << RXBSY))
		;
	
	// request standby mode
	CANGCON = 0;
	
	// wait until the CAN Controller has entered standby mode
	while (CANGSTA & (1 << ENFG))
		;
}
extern __attribute__ ((gnu_inline)) inline void _leave_standby_mode(void)
{
	// save CANPAGE register
	uint8_t canpage = CANPAGE;
	
	// reenable all MObs
	for (uint8_t i=0;i<15;i++) {
		CANPAGE = i << 4;
		CANCDMOB = CANCDMOB;
	}
	
	// restore CANPAGE
	CANPAGE = canpage;
	
	// request normal mode
	CANGCON = (1 << ENASTB);
	
	// wait until the CAN Controller has left standby mode
	while ((CANGSTA & (1 << ENFG)) == 0)
		;
}
#endif
