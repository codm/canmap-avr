#ifndef MAIN_H_
#define MAIN_H_
/*
   Standard defintions
   */
#define F_CPU 16000000UL
#define BAUD 9600UL

/*
  CAN Definitions
  */

#define CANP_REQUEST(a) ( a < 0x42 );
#define CANP_REQANSWER 0x43
#define CANP_BLINK 0x44
#define CANP_RESET 0x45
#define CANP_SYNC 0xFA
#define CANP_ACK 0xFB

#define CANBLOCKS_MAX 255
#define CANBLOCKS_START 0xFF
#define CANBLOCKS_END 0xFE

#define CANBLOCKST_STRING 0x01

#define CANBLOCKSM_STATE_READY 0x00
#define CANBLOCKSM_STATE_TRANS 0x01
#define CANBLOCKSM_STATE_FIN   0x02

#define CANBLOCKSM_SYNC_START 0x01

#define CANBLOCKSM_SYNC_TYPE_STRING 0x01

#define CAN_SELF 0x01
#define CAN_OTHER 0x02




/*
  Typedefinitions
  */

typedef struct {
    uint8_t rec;
    uint8_t send;
    uint8_t command;
    uint8_t data[6];
} CANMESSAGE;

typedef struct {
    uint8_t rec;
    uint8_t send;
    uint8_t command;
    uint8_t blocklen;
    uint32_t timer;
    char* data;
    uint8_t status; // 0: waiting, 1: transmittions, 2: ready
} CANBLOCKS_MESSAGE;
#endif

