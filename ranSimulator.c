#include "ranSimulator.h"

dataRing *sdapQueue1;
dataRing *sdapQueue2;
dataRing *macQueue;
FILE *file_fd;
int packetIn;
int packetOut;


void sendPing ( sendThreadDataPtr sentThrData )
{
    dataPkt *sentPkt;
    int sendLen;
    int pingReplLen;
    int poolCount;
    gtpUPtr gtpHdr;
    char* pingReply;
    while (1)
    {

        sentPkt = rte_pktmbuf_alloc ( sentThrData->ranSentPktPool );
        if ( NULL == sentPkt  )
        {
            printf ("Alloc failed for sent\n");
            return FAILURE;
        }

        pingReply = rte_pktmbuf_append ( sentPkt, 1500 );
        if ( NULL == pingReply )
        {
            printf ("Ping append failed\n");
            return FAILURE;
        }

        pingReplLen = read ( sentThrData->tap_fd, (void *)pingReply, 1500 );
        if ( pingReplLen < 0 )
        {
            printf ("Fail to read kernel packet\n");
            return FAILURE;
        }
        if ( FAILURE == rte_pktmbuf_trim ( sentPkt, 1500 - pingReplLen ) )
        {
            printf("Trim failed\n");
	    continue;
            return FAILURE;
        }
        if ( FAILURE == sendMsg ( sentThrData->sock, sentPkt ) )
        {
            printf ("Failed to sent data\n");
            return FAILURE;
        }
        rte_pktmbuf_free (sentPkt);
        sentPkt = NULL;
    }
}

void headerAddition1 ( )
{
    dataPkt *sdapPkt;
    char *pdcpRlcAdd;
    while(1)
    {
	if ( rte_ring_dequeue ( sdapQueue1 , &sdapPkt ) < 0 ) continue;
        pdcpRlcAdd = rte_pktmbuf_prepend ( sdapPkt, 5);
        if ( NULL == pdcpRlcAdd )
        {
            printf (" Memory allocation for pdcp rlc header failed\n");
            return;
        }
        memset ( pdcpRlcAdd, 0xaa, 5);
        if ( !rte_ring_full ( macQueue ) )
        {
            if ( rte_ring_enqueue ( macQueue, sdapPkt ) < 0 )
            {
                printf ("Enqueue failed\n");
                return;
            }
        }
        else
        {
            printf ("Packet droped");
            rte_pktmbuf_free ( sdapPkt );
            sdapPkt = NULL;
        }
    }
}

#if 0
void headerAddition2 ( )
{
    dataPkt *sdapPkt;
    char *pdcpRlcAdd;
    while(1)
    {
        while ( rte_ring_empty ( sdapQueue2 ) );
        if ( rte_ring_dequeue ( sdapQueue2, &sdapPkt) < 0 )
        {
            printf ("Dequeue failed\n");
            return;
        }
        pdcpRlcAdd = rte_pktmbuf_prepend ( sdapPkt, 5);
        if ( NULL == pdcpRlcAdd )
        {
            printf (" Memory allocation for pdcp rlc header failed\n");
            return;
        }
        memset ( pdcpRlcAdd, 0xaa, 5);
        if ( !rte_ring_full ( macQueue ) )
        {
            if ( rte_ring_enqueue ( macQueue, sdapPkt ) < 0 )
            {
                printf ("Enqueue failed\n");
                return;
            }
        }
        else
        {
            rte_pktmbuf_free ( sdapPkt );
            sdapPkt = NULL;
        }
    }
}
#endif

void macLayer ( recvThreadDataPtr recvThrData )
{
    int pingLen;
    dataPkt *rlcPdcpSdap; 
    char *macHdr;
    while(1)
    {
    	rte_ring_dump ( file_fd, sdapQueue1);
    	if ( rte_ring_dequeue (macQueue, &rlcPdcpSdap ) < 0)
	{
	     continue;
	}
	macHdr = rte_pktmbuf_prepend ( rlcPdcpSdap, 4);
	if ( NULL == macHdr )
	{
	    printf ("Mac alloc failed\n");
	    return;
	}
	memset ( macHdr, 0xcc, 9);
	if ( FAILURE == rte_pktmbuf_adj ( rlcPdcpSdap,9) )
	{
	    printf ("Adj failed\n");
	    return;
	}

        pingLen = write ( recvThrData->tap_fd, rte_pktmbuf_mtod (rlcPdcpSdap, void *), rlcPdcpSdap->pkt_len );
        if ( pingLen < 0 )
        {
            printf ("write to kernal failed %d \n", __LINE__);
            return;
        }
        rte_pktmbuf_free (rlcPdcpSdap);
        rlcPdcpSdap = NULL;
    }
}


void recvPing ( recvThreadDataPtr recvThrData )
{
    dataPkt *recvPkt;
    int pingLen;
    int poolCount;
    char *ping;
    char *sdapHdr;
    char *recvMsg1;
    int recvMsgLen;
    int len = sizeof ( recvThrData->sock->clientAddress );
    while(1)
    {

        recvPkt = rte_pktmbuf_alloc ( recvThrData->ranRecvPktPool );
        if ( NULL == recvPkt )
        {
            printf ("memory allocation failed\n");
        }

        recvMsg1 = rte_pktmbuf_append ( recvPkt, 1500 );
        if ( NULL == recvMsg )
        {
            printf("Append failed\n");
        }

        recvMsgLen = recv ( recvThrData->sock->socketFd,
                                (void * )recvMsg1,
                                1500,
                                0);
        if ( recvMsgLen < 0 )
        {
            printf ("Error in recieveing %d", errno);
        }
        
        if ( FAILURE == rte_pktmbuf_trim ( recvPkt, 1500-recvMsgLen ) )
        {
                printf ("trim failed\n");
        }


        if ( rte_ring_enqueue ( sdapQueue1, recvPkt ) < 0)
        {
            printf ("Enqueue to ring failed");
	    printf ("Packet Droped %d\n", __LINE__);
	    rte_pktmbuf_free(recvPkt);
	    recvPkt = NULL;
        }
    }
}




int createSocket ( socketParameterPtr sock )
{
    sock->socketFd = socket ( AF_INET, SOCK_DGRAM, 0);
    if ( sock->socketFd < 0 )
    {
        printf ( "Error in socket creation %d", errno );
        return FAILURE;
    }

    bzero ( &(sock->serverAddress), sizeof ( sock->serverAddress ) );
    ( &(sock->serverAddress) )->sin_family = AF_INET;
    ( &(sock->serverAddress) )->sin_port = 2152;
    ( &(sock->serverAddress) )->sin_addr.s_addr = inet_addr ( "192.168.xx.yy" );
    return SUCCESS;
}

int sendMsg ( socketParameterPtr sock, dataPkt *sendPkt )
{
    int sendPktLen;
    char *sendMsg = rte_pktmbuf_mtod ( sendPkt, void * );
    sendPktLen = sendto ( sock->socketFd,
                         (const void * )sendMsg,
                          sendPkt->pkt_len,
                          0,
                          (SA)&(sock->serverAddress),
                          sizeof( sock->serverAddress ) );
    if ( sendPktLen < 0 )
    {
        printf ("Error in sending packet %d\n", errno);
        return FAILURE;
    }
    return SUCCESS;
}

int recvMsg ( socketParameterPtr sock, dataPkt *recvPkt )
{
    char *recvMsg;
    int recvMsgLen;
    int len = sizeof ( sock->clientAddress );
    recvMsg = rte_pktmbuf_append ( recvPkt, 1500 );
    if ( NULL == recvMsg )
    {
        printf("Append failed\n");
        return FAILURE;
    }
    recvMsgLen = recvfrom (sock->socketFd,
                            (void * )recvMsg,
                            1500,
                            0,
                            (SA) &(sock->clientAddress),
                            &len);
    if ( recvMsgLen < 0 )
    {
        printf ("Error in recieveing %d", errno);
        return FAILURE;
    }
    
    if ( FAILURE == rte_pktmbuf_trim ( recvPkt, 1500-recvMsgLen ) )
    {
            printf ("trim failed\n");
            return FAILURE;
    }
    return SUCCESS;
}

int main ( int argc, char **argv )
{
    socketParameterPtr sock;
    int tap_fd; 
    dataPool *ranRecvPktPool;
    dataPool *ranSentPktPool;
dataPool *rxPool;
    dataPool *txPool;
    char tap_name[IFNAMSIZ];
    gtpUPtr gtpHeader;
    dataPkt *sendPkt;
    dataPkt *recvPkt;
    pthread_t sendThread, recvThread, macThread, headerAdd1, headerAdd2;
    recvThreadDataPtr recvThrData;
    sendThreadDataPtr sendThrData;

    /** Setting up enviroment variable */
    if ( ( rte_eal_init ( argc, argv ) ) <= FAILURE )
    {
        printf("Error in Eal initialization\n");
        return FAILURE;
    }

    file_fd = fopen ( "Queue_statastic.txt", "w+");
    if ( NULL == file_fd )
    {
	printf("Failed to open file");
	return FAILURE;
    }

    /** Allocating memory for socket parameter */
    sock = ( socketParameterPtr ) calloc ( 1, sizeof (socketParameterVar) );
    if ( NULL == sock )
    {
        printf ("Socket memory allocation failed\n");
        return FAILURE;
    }

    snprintf(tap_name, IFNAMSIZ,"tapInt%.2u",0);
    tap_fd = createTap(tap_name);
    if ( tap_fd < 0 )
    {
        printf ("Failed to create tap interface\n");
        return FAILURE;
    }

    /** Creating socket */
    if ( FAILURE == createSocket ( sock ) )
    {
        printf ("Socket creation failed\n");
        return FAILURE;
    }

    /** creating send and recieve pools */
    if ( NULL == ( ranRecvPktPool = recvPoolCreation (ranRecvPktPool) ) )
    {
        printf ("Recieve pool creation failed\n");
        return FAILURE;
    }

    if ( NULL == ( ranSentPktPool = sentPoolCreation (ranSentPktPool) ) )
    {
        printf ("Sent pool creation failed\n");
        return FAILURE;
    }

    /** Allocating memory from pool */
    sendPkt = rte_pktmbuf_alloc ( ranSentPktPool );
    if ( NULL == sendPkt )
    {
        printf ("Packet allocation failed recv\n");
        return FAILURE;
    }

    gtpHeader = rte_pktmbuf_append ( sendPkt, sizeof ( gtpUvar ) );
    if ( NULL == gtpHeader )
    {
        printf ("Memory allocation for gtp header failed\n");
        return FAILURE;
    }

    headerFill ( gtpHeader, 1 );

    if ( FAILURE == sendMsg ( sock, sendPkt ) )
    {
        printf ("Error in sending\n");
        return FAILURE;
    }
    printf ("Echo request sent\n");

    rte_pktmbuf_free ( sendPkt );
    sendPkt = NULL;

    recvPkt = rte_pktmbuf_alloc ( ranRecvPktPool );
    if ( NULL == recvPkt )
    {
        printf ("Packet allocation failed recv\n");
        return FAILURE;
    }

    if ( FAILURE == recvMsg ( sock, recvPkt ) )
    {
        printf ("Recieve failed\n");
        return FAILURE;
    }
    printf ("Echo response recieved\n");

    rte_pktmbuf_free (recvPkt);
    recvPkt = NULL;

    /** Data related to thread support */
    recvThrData = recvThreadCreate();
    if ( NULL == recvThrData )
    {
        printf ("memory allocation for send thread failed" );
        return FAILURE;
    }    

    sendThrData = sendThreadCreate();
    if ( NULL == sendThrData )
    {
        printf ("memory allocation for send thread failed" );
        return FAILURE;
    }    

    recvThrData->sock = sock;
    recvThrData->ranRecvPktPool = ranRecvPktPool;
    recvThrData->tap_fd = tap_fd;

    sendThrData->sock = sock;
    sendThrData->ranSentPktPool = ranSentPktPool;
    sendThrData->tap_fd = tap_fd;

    sdapQueue1 = rte_ring_create ( "Queue1", 32, rte_socket_id(), 0x0001 | 0x0002);
    if ( NULL == sdapQueue1 )
    {
        printf("Queue 1 creation failed\n");
	return FAILURE;
    }
    sdapQueue2 = rte_ring_create ( "Queue2", 16, rte_socket_id(), 0x0001 | 0x0002);
    if ( NULL == sdapQueue2 )
    {
        printf("Queue 2 creation failed\n");
	return FAILURE;
    }

    macQueue = rte_ring_create ( "macQueue", 1024, rte_socket_id(), 0 );
    if ( NULL == macQueue )
    {
	printf (" Failed to create mac queue\n");
	return FAILURE;
    }

    sleep(5);
    if ( rte_eal_remote_launch ( sendPing, sendThrData, 1 ) == -EBUSY )
    {
	printf ("Core launch failed\n");
	return FAILURE;
    }
    if ( rte_eal_remote_launch ( recvPing, recvThrData, 2 ) == -EBUSY )
    {
	printf ("Core launch failed\n");
	return FAILURE;
    }
#if 0
    if ( rte_eal_remote_launch ( recvPing, recvThrData, 5 ) == -EBUSY )
    {
	printf ("Core launch failed\n");
	return FAILURE;
    }
    if ( rte_eal_remote_launch ( recvPing, recvThrData, 6 ) == -EBUSY )
    {
	printf ("Core launch failed\n");
	return FAILURE;
    }
    if ( rte_eal_remote_launch ( recvPing, recvThrData, 7 ) == -EBUSY )
    {
	printf ("Core launch failed\n");
	return FAILURE;
    }
    if ( rte_eal_remote_launch ( recvPing, recvThrData, 8 ) == -EBUSY )
    {
	printf ("Core launch failed\n");
	return FAILURE;
    }
#endif 
    if ( rte_eal_remote_launch ( macLayer, recvThrData, 3 ) == -EBUSY )
    {
	printf ("Core launch failed\n");
	return FAILURE;
    }
    if ( rte_eal_remote_launch ( headerAddition1, NULL, 4 ) == -EBUSY )
    {
	printf ("Core launch failed\n");
	return FAILURE;
    }

    rte_eal_mp_wait_lcore();
    return SUCCESS;
}
