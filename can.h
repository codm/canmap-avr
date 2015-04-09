#ifndef __CAN_H__
#define __CAN_H__

#ifndef F_CPU
#define F_CPU 16000000UL
#endif


// -----------------------------------------------------------------------------
// AT90CAN128 Can function definition
// -----------------------------------------------------------------------------

extern void can_init();
extern void can_send();
extern void can_receive();

// -----------------------------------------------------------------------------
// pin and register definitions
// -----------------------------------------------------------------------------

#define CAN_PORT DDRD
#define CAN_RX PD6
#define CAN_TX PD7




#endif
