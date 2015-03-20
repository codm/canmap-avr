// coding: utf-8

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "main.h"
#include "can.h"

uint8_t self = 0x01;
uint8_t rec = 0xff;

CANBLOCKS_MESSAGE cbm;

int canblocks_receive(CANBLOCKS_MESSAGE *msg)
{
  int len = 0;
  can_t getm;
  can_get_message(&getm);
  if(getm.data[1] == CANP_SYNC) {
    if(getm.data[2] == CANBLOCKS_START) {
      canblocks_emtpy_data(msg);
    }
  }
}

int canblocks_send(CANBLOCKS_MESSAGE msg) {
  can_t sendmsg;
  msg.send = self;
  msg.rec = rec;
  msg.data = "herzliches Hallo vom Sender an den Empfaenger";
  msg.command = CANP_SYNC;
  msg.blocklen = sizeof(msg.data)/sizeof(uint8_t);
  msg.timer = 0;
  msg.status = CANBLOCKSM_STATE_READY;

  sendmsg.id = msg.rec;
  sendmsg.data[0] = msg.send;
  sendmsg.data[1] = msg.command;
  if(msg.command == CANP_SYNC) 
  {
    while(*dataptr != '\0') 
    {
      int iter_send;
      for(iter_send = 2; iter_send < 8; iter_send++) 
      {
        sendmsg.data[iter_send] = dataptr++;
      }
    }
  }
  else
  {
    int iter_send;
    for(iter_send = 2; iter_send < 8; iter_send++)
    {
      sendmsg.data[iter_send] = msg.data[iter_send - 2];
    }
  }
  can_send_message(&sendmsg);
}

}

void canblocks_emtpy_data(CANBLOCKS_MESSAGE *msg) {
  msg->status = ready;
  msg->timer = 0;
  msg->blocklen = 0;
  msg->command = 0;
  msg->send = 0;
  msg->rec = 0;
  for(i = 0; i++; i < CANBLOCKS_MAX) {
    msg->data[i] = 0;
  }
}

// -----------------------------------------------------------------------------
// Main loop for receiving and sending messages.

int main(void)
{
  // Initialize MCP2515
  can_init(BITRATE_125_KBPS);
  can_send_message(&msg);
  while (1)
  {
    canblocks_receive(cbm);
  }
  return 0;
}

