#include "canblocks.h"
#include "uart.h"

can_t getm;
canblocksmsg_t *gmsg;
canblocksmsg_t cbl_buffer[CANBLOCKS_BUFFER_SIZE];

int _buffer_get_free_slot(canblocksmsg_t **msg);
void _buffer_copy_values(canblocksmsg_t *dst, canblocksmsg_t *src);
int _buffer_get_sender(canblocksmsg_t **msg, uint8_t sender);

void canblocks_reset_data(canblocksmsg_t *msg) {
    int i;
    msg->send = 0;
    msg->rec = 0;
    msg->command = 0;
    msg->status = CANBLOCKSM_STATE_READY;
    msg->blocklen = 0;
    msg->type = CANBLOCKSM_TYPE_NORMAL;
    for(i = 0; i < CANBLOCKS_DATA_MAX; i++) {
        msg->blockdata[i] = '\0';
    }
    msg->data = &msg->blockdata[0];
    for(i = 0; i < 6; i++) {
        msg->singledata[i] = 0;
    }
    msg->timer = 0;
}

int canblocks_receive(canblocksmsg_t *msg) {
    // int len = 0;
    if(can_check_message()) {
        if(can_get_message(&getm)) {
            if(getm.data[1] == CANP_SYNC) {
                /* only if this is start header */
                if(getm.data[2] == CANBLOCKS_SOH) {
                    if(!_buffer_get_free_slot(&gmsg))
                        return 0;
                    gmsg->rec = getm.id;
                    gmsg->send = getm.data[0];
                    gmsg->command = getm.data[1];
                    gmsg->blocklen = getm.data[6];
                    gmsg->type = getm.data[7];
                    gmsg->status =  CANBLOCKSM_STATE_TRANS;
                    gmsg->data = &gmsg->blockdata[0];
                    return 2;
                }

                /* else look for the used buffer */
                if(_buffer_get_sender(&gmsg, getm.data[0])) {
                    int iter;
                    for(iter = 2; iter < 8; iter++) {
                        if(getm.data[iter] == CANBLOCKS_EOT) {
                            *gmsg->data = '\0';
                            gmsg->status = CANBLOCKSM_STATE_FIN;
                            _buffer_copy_values(msg, gmsg);
                            canblocks_reset_data(gmsg);
                            msg->data = &msg->blockdata[0];
                            return 1;
                        }
                        *gmsg->data = getm.data[iter];
                        gmsg->data++;
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
        return 9; /* error while getting message */
    }
    return 0; /* no message available */
}

int canblocks_send(canblocksmsg_t *msg) {
    char *dataptr;
    size_t datasize;
    can_t sendmsg;
    int iter_send, end;
    datasize = strlen(msg->data);
    if(datasize%6 != 0)
        msg->blocklen = (datasize/6)+1;
    else
        msg->blocklen = datasize/6;
    msg->timer = 0;

    sendmsg.id = msg->rec;
    sendmsg.flags.rtr = 0;
    sendmsg.flags.extended = 0;
    sendmsg.length = 8;
    sendmsg.data[0] = msg->send;
    sendmsg.data[1] = msg->command;
    dataptr = &msg->data[0];

    end = 0;
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

        while(*dataptr != '\0' && !end)
        {
            sendmsg.length = 8;
            for(iter_send = 2; iter_send < 8 && !end; iter_send++)
            {
                sendmsg.data[iter_send] = (uint8_t)*dataptr;
                dataptr++;
                if(*dataptr == '\0')
                {
                    iter_send++;
                    sendmsg.data[iter_send] = CANBLOCKS_EOT;
                    end = 1;
                }
            }
            sendmsg.length = iter_send;
            can_send_message(&sendmsg);
            _delay_ms(CANBLOCKS_DELAY);
        }
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

int _buffer_get_free_slot(canblocksmsg_t **msg) {
    int i;
    for(i = 0; i < CANBLOCKS_BUFFER_SIZE; i++) {
        if(cbl_buffer[i].status == CANBLOCKSM_STATE_READY) {
            *msg = &cbl_buffer[i];
            return 1;
        }
    }
    return 0;
}

void _buffer_copy_values(canblocksmsg_t *dst, canblocksmsg_t *src) {
    int i;
    dst->send = src->send;
    dst->rec = src->rec;
    dst->command = src->command;
    dst->status = src->status;
    dst->blocklen = src->blocklen;
    dst->type = src->type;
    dst->timer = src->timer;
    for(i = 0; i < CANBLOCKS_DATA_MAX; i++) {
        dst->blockdata[i] = src->blockdata[i];
    }
    for(i = 0; i < 6; i++) {
        dst->singledata[i] = src->singledata[i];
    }
}

int _buffer_get_sender(canblocksmsg_t **msg, uint8_t sender) {
    int i;
    for(i = 0; i < CANBLOCKS_BUFFER_SIZE; i++) {
        if(cbl_buffer[i].send == sender
            && cbl_buffer[i].status == CANBLOCKSM_STATE_TRANS) {
            *msg = &cbl_buffer[i];
            return 1;
        }
    }
    return 0;
}

