#include <rte_mbuf.h>

#ifndef __POOL_CREATION_H__
#define __POOL_CREATION_H__

#define SUCCESS 0
#define FAILURE -1

typedef struct rte_mempool dataPool;
typedef struct rte_mbuf dataPkt;

dataPool* recvPoolCreation ( dataPool *ranRecvPktPool );
dataPool* sentPoolCreation ( dataPool *ranSentPktPool );

#endif /** __POOL_CREATION_H__ */