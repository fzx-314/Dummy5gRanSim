#include "thread.h"

recvThreadDataPtr recvThreadCreate()
{
    recvThreadDataPtr recvThrData;
    recvThrData = ( recvThreadDataPtr ) calloc ( 1, sizeof ( recvThreadDataVar ) );
    if ( NULL == recvThrData )
    {
        printf ("memory allocation for recv thread failed" );
        return NULL;
    }
    return recvThrData;
}

sendThreadDataPtr sendThreadCreate()
{
    sendThreadDataPtr sendThrData;
    sendThrData = ( sendThreadDataPtr ) calloc ( 1, sizeof ( sendThreadDataVar ) );
    if ( NULL == sendThrData )
    {
        printf ("Memory for send thread failed\n");
        return NULL;
    }
    return sendThrData;
}
