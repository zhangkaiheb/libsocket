#ifndef __RAW_COMM_H__
#define __RAW_COMM_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

#include<linux/if.h>
#include<linux/if_packet.h>
#include<linux/if_ether.h>
#include<linux/sockios.h>


typedef struct  
{  
	uint8_t ip_hl:4;
	uint8_t ip_v:4;  
	uint8_t ip_tos; 
	uint16_t ip_len;
	uint16_t ip_id;  
	uint16_t ip_off;  
	uint8_t ip_ttl;  
	uint8_t ip_p;  
	uint16_t ip_sum;  
	uint32_t ip_src;  
	uint32_t ip_dst;  
} __attribute__ ((packed)) ip_hdr; 

typedef struct  
{  
	uint16_t source;  
	uint16_t dest;  
	uint16_t len;  
	uint16_t check;  
} __attribute__ ((packed)) udp_hdr;  

typedef struct  
{  
    uint32_t src_ip;  
    uint32_t dst_ip;  
    uint8_t zero;  
    uint8_t protocol;  
    uint16_t udp_len;  
} __attribute__ ((packed)) psd_hdr;  


typedef struct {
	uint16_t tcph_srcport;
	uint16_t tcph_destport;
	uint32_t tcph_seqnum;
	uint32_t tcph_acknum;
	uint16_t tcph_reserved:4,
			 tcph_offset:4,
			 tcph_fin:1,
			 tcph_syn:1,
			 tcph_rst:1,
			 tcph_psh:1,
			 tcph_ack:1,
			 tcph_urg:1,
			 tcph_res2:2;
	uint16_t tcph_win;
	uint16_t tcph_chksum;
	uint16_t tcph_urgptr;
} __attribute__ ((packed)) tcp_hdr;

#endif

