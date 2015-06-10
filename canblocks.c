#include "canblocks.h"

void canblocks_reset_data(CANBLOCKS_MESSAGE *msg) {
    int i;
    msg->send = 0;
    msg->rec = 0;
    msg->command = 0;
    msg->status = CANBLOCKSM_STATE_READY;
    msg->blocklen = 0;
    msg->data = &msg->blockdata[0];
    for(i = 0; i < CANBLOCKS_DATA_MAX; i++) {
        msg->blockdata[i] = 0;
    }
    for(i = 0; i < 6; i++) {
        msg->singledata[i] = 0;
    }
    msg->timer = 0;
}
int canblocks_receive(CANBLOCKS_MESSAGE *msg) {
    // int len = 0;
    can_t getm;
    if(can_check_message()) {
        if(can_get_message(&getm)) {
            if(getm.data[1] == CANP_SYNC) {
                if(getm.data[2] == CANBLOCKS_SOH) {
                    msg->rec = getm.id;
                    msg->send = getm.data[0];
                    msg->command = getm.data[1];
                    msg->blocklen = getm.data[6];
                    msg->type = getm.data[7];
                    msg->status =  CANBLOCKSM_STATE_TRANS;
                    msg->data = &msg->blockdata[0];
                    return 2;
                }
                else if(msg->status == CANBLOCKSM_STATE_TRANS) {
                    int iter;
                    for(iter = 2; iter < 8; iter++) {
                        if(getm.data[iter] == CANBLOCKS_EOT) {
                            *msg->data = '\0';
                            msg->data = &msg->blockdata[0];
                            msg->status = CANBLOCKSM_STATE_FIN;
                            return 1;
                        }
                        *msg->data = getm.data[iter];
                        msg->data++;
                    }
                    return 2;
                }
                else {
                    return 0;
                }
            }
            else {
                int iter;
                msg->rec = getm.id;
                msg->send = getm.data[0];
                msg->command = getm.data[1];
                msg->blocklen = 0;
                msg->type = CANBLOCKSM_TYPE_NORMAL;
                msg->status = CANBLOCKSM_STATE_FIN;
                for(iter = 2; iter < 8; iter++) {
                    msg->singledata[iter-2] = getm.data[iter];
                }
                return 1;
            }
        }
        return 0; /* error while getting message */
    }
    return 0; /* no message available */
}
int canblocks_send(CANBLOCKS_MESSAGE *msg) {
    char *dataptr;
    size_t datasize;
    can_t sendmsg;
    int iter_send, null_in_for;
    msg->send = canid_self;
    msg->rec = 0xAB;
    msg->data = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    datasize = strlen(msg->data);
    msg->command = CANBLOCKS_PREFIX;
    if(datasize%6 != 0)
        msg->blocklen = (datasize/6)+1;
    else
        msg->blocklen = datasize/6;
    msg->timer = 0;
    msg->status = CANBLOCKSM_STATE_READY;

    sendmsg.id = msg->rec;
    sendmsg.length = 8;
    sendmsg.data[0] = msg->send;
    sendmsg.data[1] = msg->command;
    dataptr = &msg->data[0];

    null_in_for = 0;
    if(msg->command == CANBLOCKS_PREFIX)
    {
        /* Send CAN_SYNC startsequence */
        sendmsg.data[2] = CANBLOCKS_SOH;
        sendmsg.data[3] = 0x00;
        sendmsg.data[4] = 0x00;
        sendmsg.data[5] = 0x00;
        sendmsg.data[6] = msg->blocklen;
        sendmsg.data[7] = CANBLOCKSM_TYPE_STRING;

        can_send_message(&sendmsg);

        _delay_ms(CANBLOCKS_DELAY);

        while(*dataptr != '\0' && !null_in_for)
        {
            for(iter_send = 2; iter_send < 8; iter_send++)
            {
                if(null_in_for) {
                    sendmsg.data[iter_send] = 0x00;
                } else {
                    sendmsg.data[iter_send] = (uint8_t)*dataptr;
                }
                dataptr++;
                if(*dataptr == '\0')
                {
                    iter_send++;
                    sendmsg.data[iter_send] = CANBLOCKS_ETX;
                    null_in_for = 1;
                }
            }
            can_send_message(&sendmsg);
            _delay_ms(CANBLOCKS_DELAY);
        }

        /* Send CAN_SYNC Endsequence */
        for(iter_send = 2; iter_send < 8; iter_send++) {
            sendmsg.data[iter_send] = 0x00;
        }
        can_send_message(&sendmsg);
    }
    else
    {
        int iter_send;
        for(iter_send = 2; iter_send < 8; iter_send++) {
            sendmsg.data[iter_send] = msg->singledata[iter_send - 2];
        }
        can_send_message(&sendmsg);
    }
    return 0;
}
