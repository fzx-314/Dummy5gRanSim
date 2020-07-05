#include <linux/if_tun.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <stdlib.h>
#include <string.h>

#ifndef __TAP_H__
#define __TAP_H__

int createTap ( char *name );

#endif /** __TAP_H__ */
/** EOF */