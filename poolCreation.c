#include "poolCreation.h"

dataPool* recvPoolCreation ( dataPool *ranRecvPktPool )
{
    ranRecvPktPool =  rte_pktmbuf_pool_create ( "recvPool", 2048, 256, 0, 8192, rte_socket_id () );
    if ( NULL == ranRecvPktPool )
    {
        printf ( "Memory pool allocation failed" );
        return NULL;
    }
    return ranRecvPktPool;
}

dataPool* sentPoolCreation ( dataPool *ranSentPktPool )
{
    ranSentPktPool =  rte_pktmbuf_pool_create ( "sentPool", 2048, 256, 0, 8192, rte_socket_id () );
    if ( NULL == ranSentPktPool )
    {
        printf ( "Memory pool allocation failed" );
        return NULL;
    }
    return ranSentPktPool;
}