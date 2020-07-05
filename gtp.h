#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 

#ifndef __GTP_H__
#define __GTP_H__

/** Extenstion header structure */
typedef struct __attribute__((packed)) extHdrCntn
{
    u_int8_t length;
    u_int8_t pduSessnCntr;
    u_int8_t nxtHdr;
} extHdrCntnVar,
 *extHdrCntnPtr;

/** GTP header structure */
typedef struct __attribute__((packed)) gtpU
{
    u_int8_t nextPduNumFlag:1;
    u_int8_t seqNumFlag:1;
    u_int8_t extendHdrFlag:1;
    u_int8_t spare:1;
    u_int8_t protocolType:1;
    u_int8_t version:3;
    u_int8_t messageType;
    u_int16_t length;
    u_int32_t teid;
    u_int16_t seqNum;
    u_int8_t nextPduNum;
    u_int8_t nxtExtHdr;
} gtpUvar,
 *gtpUPtr;

 void headerFill ( gtpUPtr hdr, int msgType );

#endif /** __GTP_H__ */
/** EOF */