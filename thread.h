#include <pthread.h>
#include <stdlib.h>
#include <arpa/inet.h>


#ifndef __THREAD_H__
#define __THREAD_H__

#define SUCCESS 0
#define FAILURE -1

typedef struct rte_mempool dataPool;
typedef struct rte_mbuf dataPkt;

typedef struct socketParameter
{
    int socketFd;
    struct  sockaddr_in serverAddress;
    struct  sockaddr_in clientAddress;
} socketParameterVar,
 *socketParameterPtr;

typedef struct recvThreadData 
{
    socketParameterPtr sock;
    dataPool *ranRecvPktPool;
    int tap_fd;
} recvThreadDataVar,
 *recvThreadDataPtr;

 typedef struct sendThreadData
 {
     socketParameterPtr sock;
     dataPool *ranSentPktPool;
     int tap_fd;
 } sendThreadDataVar,
  *sendThreadDataPtr;

recvThreadDataPtr recvThreadCreate();
sendThreadDataPtr sendThreadCreate();

#endif /** __THREAD_H__ */
/** EOF */