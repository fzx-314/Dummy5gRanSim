#include "upf.h"

void recvPing ( recvThreadDataPtr recvThrData )
{
    dataPkt *recvPkt;
    int pingLen;
    while(1)
    {
        recvPkt = rte_pktmbuf_alloc ( recvThrData->ranRecvPktPool );
        if ( NULL == recvPkt )
        {
            printf ("Alloc failed\n");
            return;
        }
 
        if ( FAILURE == recvMsg (recvThrData->sock, recvPkt) )
        {
            printf ("fail to recieve\n");
            return;
        }

        /** Removing GTP header */
        if ( NULL == rte_pktmbuf_adj (recvPkt, 12 ) )
        {
            printf ("Adj failed\n");
            return FAILURE;
        }

        /** Writing packet back to kernal */
        pingLen = write ( recvThrData->tap_fd, rte_pktmbuf_mtod (recvPkt, void *), recvPkt->pkt_len );
        if ( pingLen < 0 )
        {
            printf ("write to kernal failed\n");
            return FAILURE;
        }
        rte_pktmbuf_free (recvPkt);
        recvPkt = NULL;
    }
}

void sendPing ( sendThreadDataPtr sendThrData )
{
    dataPkt *sendPkt;
    gtpUPtr gtpHdr;
    char *pingRequest;
    int pingReqLen;
    while (1)
    {
        sendPkt = rte_pktmbuf_alloc ( sendThrData->ranSentPktPool );
        if ( NULL == sendPkt )
        {
            printf ("Alloc failed\n");
            return FAILURE;
        }
        gtpHdr = rte_pktmbuf_append ( sendPkt, sizeof (gtpUvar) );
        if ( NULL == pingRequest )
        {
            printf ("Append failed\n");
            return FAILURE;
        }

        headerFill ( gtpHdr, 255 );
        pingRequest = rte_pktmbuf_append ( sendPkt, 1500 );
        if ( NULL == pingRequest )
        {
            printf ("Ping append faild\n");
            return FAILURE;
        }
        pingReqLen = read ( sendThrData->tap_fd, (void *)pingRequest, 1500 );
        if ( pingReqLen < 0 )
        {
            printf ("Failed to read kernel packet\n");
            return FAILURE;
        }

        if ( FAILURE == rte_pktmbuf_trim (sendPkt, 1500 - pingReqLen) )
        {
            printf ("Trim failed\n");
            return FAILURE;
        }

        if ( FAILURE == sendMsg ( sendThrData->sock, sendPkt ) )
        {
            printf ("send failed\n");
            return FAILURE;
        }
        rte_pktmbuf_free (sendPkt);
        sendPkt = NULL;
    }
    
}

int sendMsg ( socketParameterPtr sock, dataPkt *sendPkt )
{
    int sendPktLen;
    char *sendMsg = rte_pktmbuf_mtod ( sendPkt, void * );
    sendPktLen = sendto ( sock->socketFd,
                         (const void * )sendMsg,
                          sendPkt->pkt_len,
                          0,
                          (struct sockaddr *)&(sock->serverAddress),
                          sizeof( struct sockaddr ) );
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
    recvMsgLen = recvfrom ( sock->socketFd, recvMsg, 1500, 0,
                            (struct sockaddr *)&(sock->clientAddress ), &len );
    if ( recvMsgLen < 0 )
    {
        printf ("Failed to recieve message %d\n", errno);
        return FAILURE;
    }

    if ( FAILURE == rte_pktmbuf_trim (recvPkt, 1500-recvMsgLen) )
    {
        printf ("Trim failed\n");
        return FAILURE;
    }
    return SUCCESS;
}

int socketCreate ( socketParameterPtr sockParam )
{
    sockParam->socketFd = socket ( AF_INET, SOCK_DGRAM, 0 );
    if ( sockParam->socketFd < 0 )
    {
        printf ( "Error in socket %d", errno );
        return FAILURE;
    }
    /** intializing server parameters */
    bzero ( &(sockParam->serverAddress), sizeof ( sockParam->serverAddress ) );
    ( &(sockParam->serverAddress) )->sin_family = AF_INET;
    ( &(sockParam->serverAddress) )->sin_port = GTP_U_PORT;
    ( &(sockParam->serverAddress) )->sin_addr.s_addr = INADDR_ANY;

    /** Binding server */
    if ( bind ( sockParam->socketFd, 
                ( const struct sockaddr * )&(sockParam->serverAddress), 
                 sizeof ( sockParam->serverAddress ) ) )
    {
        printf (" Bind failed %d", errno );
        return FAILURE;
    }
    return SUCCESS;
}

int main ( int argc, char **argv )
{
    char tap_name[IFNAMSIZ];
    gtpUPtr *echoResponse;
    int tap_fd;
    dataPool *corePktRecvPool;
    dataPool *coreSentRecvPool;
    dataPkt *recvPkt;
    dataPkt *sentpkt;
    socketParameterPtr sockParam;
    pthread_t sendThread, recvThread;
    recvThreadDataPtr recvThrData;
    sendThreadDataPtr sendThrData;


    if ( ( rte_eal_init ( argc, argv ) ) < FAILURE )
    {
        printf( "Error in intit\n" );
        return FAILURE;
    }

    snprintf(tap_name, IFNAMSIZ,"tapInt%.2u",0);
    tap_fd = createTap(tap_name);

    if ( tap_fd < 0 )
    {
        printf ("Failed to create Tap interface\n");
        return FAILURE;
    }

    sockParam = ( socketParameterPtr ) calloc ( 1, sizeof (socketParameterVar) );
    if ( NULL == sockParam )
    {
        printf ("Memory allocation of socket parameter failed\n");
        return FAILURE;
    }

    if ( FAILURE == socketCreate (sockParam) )
    {
        printf ("socket creation failed");
        return FAILURE;
    }

    if ( NULL == ( corePktRecvPool = recvPoolCreation ( corePktRecvPool ) ) )
    {
        printf ("Recieve packet pool creation failed\n");
        return FAILURE;
    }

    if ( NULL == ( coreSentRecvPool = sentPoolCreation ( coreSentRecvPool ) ) )
    {
        printf ("Sent pool creation failed\n");
        return FAILURE;
    }

    recvPkt = rte_pktmbuf_alloc ( corePktRecvPool );
    if ( NULL == recvPkt )
    {
        printf ("Alloc failed\n");
        return FAILURE;
    }

    if ( FAILURE == recvMsg ( sockParam, recvPkt ) )
    {
        printf ("Recieve failed\n");
        return FAILURE;
    }
    printf ("Echo request sent\n");

    rte_pktmbuf_free (recvPkt);
    recvPkt = NULL;

    sentpkt = rte_pktmbuf_alloc ( coreSentRecvPool );
    if ( NULL == sentpkt )
    {
        printf ("Alloc failrd\n");
        return FAILURE;
    }
    echoResponse = rte_pktmbuf_append ( sentpkt, sizeof ( gtpUvar ) );
    if ( NULL == echoResponse )
    {
        printf ("Fail to append memory\n");
        return FAILURE;
    }

    headerFill ( echoResponse, 2 );

    if ( FAILURE == sendMsg (sockParam, sentpkt) )
    {
        printf ("Failed to send messgae\n");
        return FAILURE;
    }
    printf ("Echo response sent\n");
    rte_pktmbuf_free (sentpkt);
    sentpkt = NULL;


    recvThrData = ( recvThreadDataPtr ) calloc ( 1, sizeof (recvThreadDataVar) );
    if ( NULL == recvThrData )
    {
        printf ("Unable to allocate memory\n");
        return FAILURE;
    }

    sendThrData = ( sendThreadDataPtr ) calloc ( 1, sizeof (sendThreadDataVar) );
    if ( NULL == sendThrData )
    {
        printf ("Unable to allocate memory\n");
        return FAILURE;
    }

    recvThrData->sock = sockParam;
    recvThrData->ranRecvPktPool = corePktRecvPool;
    recvThrData->tap_fd = tap_fd;

    sendThrData->sock = sockParam;
    sendThrData->ranSentPktPool = coreSentRecvPool;
    sendThrData->tap_fd = tap_fd;

    pthread_create ( &sendThread, NULL, sendPing, sendThrData );
    pthread_create ( &recvThread, NULL, recvPing, recvThrData );
    pthread_join ( sendThread, NULL );

    return SUCCESS;
}