# cod.m CANBLOCKS-AVR

This Project is an AT90CAN based CANBLOCKS implementation. The CANBLOCKS protocol specification can be viewed under -TODO-

## Base

This Implementation is based on Fabian Greiffs avr-can-lib (https://github.com/dergraaf/avr-can-lib). It receives can_t messages by Fabians library and organizes them via the canblocks_compute_frame function into an internal data struct.

I am very aware that this method is absolutely not performant. But since it's okay for our application and a lot less work that implementing a library by ourself, we decided to use Fabians avr-can-lib as a base.

This Project is build and tested for AT90CAN128. If you want to use it with another controller (like ATMega with MCP2515), it should be enough to recompile the avr-can-lib and replace can.h/config.h/libcan.a in this package.