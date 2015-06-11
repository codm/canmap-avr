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

#define CANBLOCKS_PREFIX 0xFA
#define CANBLOCKS_MAX 255
#define CANBLOCKS_SOH 0x01
#define CANBLOCKS_STX 0x02
#define CANBLOCKS_ETX 0x03
#define CANBLOCKS_EOT 0x04
#define CANBLOCKS_DELAY 5

#define CANBLOCKSM_STATE_READY 0x00
#define CANBLOCKSM_STATE_TRANS 0x01
#define CANBLOCKSM_STATE_FIN   0x02

#define CANBLOCKSM_TYPE_NORMAL 0x00
#define CANBLOCKSM_TYPE_STRING 0x01

#define CAN_SELF 0x01
#define CAN_OTHER 0x02

#define CAN_ALL_FILTER 0xff
#define CANBLOCKS_DATA_MAX 10*6 /* max 10 blocks a 6 chars */
#define CANBLOCKS_BUFFER_SIZE 2


/* Typedefinitions */
typedef struct {
    uint8_t send; /* Sender */
    uint8_t rec; /* Receiver */
    uint8_t command; /* command */
    uint8_t status; /* Status */
    uint8_t blocklen; /* Number of 8bit Blocks */
    uint8_t type; /* type of message (CANBLOCKSM_TYPE...) */
    uint8_t singledata[6]; /* if just one message */
    char* data; /* Full Data Blocklength */
    char  blockdata[CANBLOCKS_DATA_MAX];
    uint32_t timer; /* Timer */
} canblocksmsg_t;


/* Prototypes */
extern void canblocks_reset_data(canblocksmsg_t *msg);
extern int canblocks_receive(canblocksmsg_t *msg);
extern int canblocks_send(canblocksmsg_t *msg);

/* Global Variables */
#endif

