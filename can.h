#ifndef CAN_H_
#define CAN_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

// -----------------------------------------------------------------------------
// pin and register definitions
// -----------------------------------------------------------------------------


#define CAN_PORT DDRD
#define CAN_RX PD6
#define CAN_TX PD7

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


// -----------------------------------------------------------------------------
// AT90CAN128 Can function definition
// -----------------------------------------------------------------------------

extern int can_init(uint8_t bitrate);
extern int can_send(CANMESSAGE msg);
#endif
