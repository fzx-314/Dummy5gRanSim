#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>


#include <rte_eal.h>
#include <rte_mbuf.h>
#include <rte_ring.h>
#include <rte_launch.h>

#include "poolCreation.h"
#include "tap.h"
#include "gtp.h"
#include "thread.h"

#ifndef __RAN_SIMULATOR_H__
#define __RAN_SIMULATOR_H__

typedef struct sockaddr* SA;

#define GTP_U_PORT 2152


typedef struct rte_mempool dataPool;
typedef struct rte_mbuf dataPkt;
typedef struct rte_ring dataRing;

void sendPing ( sendThreadDataPtr sentThrData );
void recvPing ( recvThreadDataPtr recvThrData );
int createSocket ( socketParameterPtr sock );
int sendMsg ( socketParameterPtr sock, dataPkt *sendPkt );
int recvMsg ( socketParameterPtr sock, dataPkt *recvPkt );

#endif /** __RAN_SIMULATOR_H__ */
/** EOF */
