#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/types.h>
#include <linux/netfilter.h>		
#include <libnetfilter_queue/libnetfilter_queue.h>

#ifndef __IPTABLES_H__
#define __IPTABLES_H__

static u_int32_t print_pkt (struct nfq_data *tb);
int cb(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data);

#endif /** __IPTABLES_H__ */
