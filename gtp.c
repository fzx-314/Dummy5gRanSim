#include "gtp.h"

void headerFill ( gtpUPtr hdr, int msgType )
{
    hdr->nextPduNum = 0;
    hdr->seqNumFlag = 0;
    hdr->extendHdrFlag = 0;
    hdr->spare = 0;
    hdr->protocolType =1;
    hdr->version =1;
    hdr->messageType = msgType;
    hdr->length = 12;
    hdr->teid = 0x00000001;
    hdr->seqNum = 0;
    hdr->nxtExtHdr = 0;
    hdr->nextPduNum = 0;
}