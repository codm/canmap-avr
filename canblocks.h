#ifndef CANBLOCKS_H_
#define CANBLOCKS_H_

#include "can.h"
#include "globals.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

/* Defines */
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
#define CANBLOCKS_DATA_MAX 64*8


/* Typedefinitions */
typedef struct {
    uint8_t send; /* Sender */
    uint8_t rec; /* Receiver */
    uint8_t command; /* command */
    uint8_t status; /* Status */
    uint8_t blocklen; /* Number of 8bit Blocks */
    uint32_t timer; /* Timer */
    char* data; /* Full Data Blocklength */
} CANBLOCKS_MESSAGE;


/* Prototypes */
extern void canblocks_emtpy_data(CANBLOCKS_MESSAGE *msg);
extern int canblocks_receive(CANBLOCKS_MESSAGE *msg);
extern int canblocks_send(CANBLOCKS_MESSAGE *msg);

/* Global Variables */
CANBLOCKS_MESSAGE canblocks_message;
#endif

