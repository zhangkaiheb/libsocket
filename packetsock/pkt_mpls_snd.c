/*
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>

//#define ETH_ADDR_LEN	6
//#define ETH_HDR_LEN	14

//#define ETH_P_MPLS	0x8847	//MPLS type
//#define ETH_P_IP	0x0800	//IP protocol

#define MPLS_LABEL_LEN	20
#define MPLS_COS_LEN	3
//#define MPLS_S_BIT	1
//#define MPLS_TTL_LEN	

#define BUF_SIZ		2048

#pragma pack(push,1)
struct mpls_header{
    u_int8_t label_part_1;
    u_int8_t label_part_2;
    u_int8_t exp_s:4;
    u_int8_t label_part_3:4;
    u_int8_t ttl;
} __attribute__((packed));
#pragma pack(pop)

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


static void usage()
{
	printf("pkt_mpls_snd interface src-ip dst-mac dst-ip\n");
	exit(0);
}

static void merror(char *err)
{
	perror(err);
	exit(-1);
}

static uint16_t check_sum(void *data, size_t len)
{
	uint16_t *p = (uint16_t *)data;
	size_t left = len;
	uint32_t sum = 0;
	while (left > 1) {
		sum += *p++;
		left -= sizeof(uint16_t);
	}
	if (left == 1) {
		sum += *(uint8_t *)p;
	}
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	return ~sum;
}


int main(int argc, char *argv[])
{
    int sockfd = -1;
    struct ifreq if_idx;
    struct ifreq if_mac;
    int tx_len = 0;
    char sendbuf[BUF_SIZ];
    struct ether_header *eh = (struct ether_header *) sendbuf;
    struct mpls_header  *mh = (struct mpls_header *) (sendbuf + sizeof(struct ether_header));
    struct sockaddr_ll socket_address;
	char ifName[IFNAMSIZ];
	char *data;
	ip_hdr *ip_header;
	udp_hdr *udp_header;
	psd_hdr *pseudo_header;
	int udp_len, total_len;
	char dst_ip[64] = "10.0.0.1";
	char src_ip[64] = "192.168.1.10";
	uint32_t src_ip_v = 0;
	uint32_t dst_ip_v = 0;
	int i, dlen = 4;
	int udp_check_len;
	uint8_t dst_mac[6] = {0x00, 0x90, 0x27, 0xfe, 0xd3, 0x42};


    /* Get interface name */
	if (argc > 1)
		strcpy(ifName, argv[1]);
	else
		usage();

	if (argc > 2)
		strncpy(src_ip, argv[2], sizeof(src_ip)-1);
	if (argc > 3) {
		int m[6];
		int n = sscanf(argv[3], "%x:%x:%x:%x:%x:%x", 
					&m[0], &m[1], &m[2], &m[3], &m[4], &m[5]);
		if (n != 6)
        	merror("dst mac");
		for (i=0; i<6; i++)
			dst_mac[i] = (m[i] & 0xff);
	}
	if (argc > 4)
		strncpy(dst_ip, argv[4], sizeof(dst_ip)-1);

    /* Open RAW socket to send on */
    if ((sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
        merror("socket");
    }

    /* Get the index of the interface to send on */
    memset(&if_idx, 0, sizeof(struct ifreq));
    strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);

    if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
        merror("SIOCGIFINDEX");

    /* Get the MAC address of the interface to send on */
    memset(&if_mac, 0, sizeof(struct ifreq));
    strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);

    if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0)
        merror("SIOCGIFHWADDR");

    /* Construct the Ethernet header */
    memset(sendbuf, 0, BUF_SIZ);
    /* Ethernet header */
    eh->ether_shost[0] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
    eh->ether_shost[1] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
    eh->ether_shost[2] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
    eh->ether_shost[3] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
    eh->ether_shost[4] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
    eh->ether_shost[5] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5];
    eh->ether_dhost[0] = dst_mac[0];
    eh->ether_dhost[1] = dst_mac[1];
    eh->ether_dhost[2] = dst_mac[2];
    eh->ether_dhost[3] = dst_mac[3];
    eh->ether_dhost[4] = dst_mac[4];
    eh->ether_dhost[5] = dst_mac[5];
    /* Ethertype field */
    eh->ether_type = htons(0x8847);
    tx_len += sizeof(struct ether_header);
    /* MPLS fiels */
    mh->label_part_1=1;
    mh->label_part_2=1;
    mh->label_part_3=1;
    mh->exp_s=1;
    mh->ttl=255;
    tx_len += sizeof(struct mpls_header);


    /* IP UDP header */
	ip_header = (ip_hdr *)(sendbuf + tx_len);
	udp_header = (udp_hdr *)(ip_header + 1);
	tx_len += sizeof(*ip_header) + sizeof(*udp_header);

	data = (char *)(udp_header + 1);
	pseudo_header = (psd_hdr *)((char *)udp_header - sizeof(psd_hdr));

	if (sizeof(*ip_header) + sizeof(*udp_header) +
			tx_len + dlen + (dlen % 2) > sizeof(sendbuf)) {
		return -1;
	}

	if (inet_pton(AF_INET, src_ip, &src_ip_v) <= 0)
		return -1;
	if (inet_pton(AF_INET, dst_ip, &dst_ip_v) <= 0)
		return -1;

	udp_len = sizeof(*udp_header) + dlen;
	total_len = sizeof(*ip_header) + sizeof(*udp_header) + dlen;

	pseudo_header->src_ip = src_ip_v;
	pseudo_header->dst_ip = dst_ip_v;
	pseudo_header->zero = 0;
	pseudo_header->protocol = IPPROTO_UDP;
	pseudo_header->udp_len = htons(udp_len);

	udp_header->source = htons(5000);
	udp_header->dest = htons(2000);
	udp_header->len = htons(udp_len);
	udp_header->check = 0;

	/* Packet data */
	for (i=0; i<dlen; i++)
		data[i] = 'a' + i;
	tx_len += dlen;

	udp_check_len = sizeof(*pseudo_header) + sizeof(*udp_header) + dlen;
	if (dlen % 2 != 0) {
		udp_check_len += 1;
		*(data + dlen) = 0;
		tx_len += 1;
	}
	udp_header->check = check_sum(pseudo_header, udp_check_len);

	ip_header->ip_hl = sizeof(*ip_header) / sizeof (uint32_t);
	ip_header->ip_v = 4;
	ip_header->ip_tos = 0;
	ip_header->ip_len = htons(total_len);
	ip_header->ip_id = htons(0); //set by protocol stack 
	ip_header->ip_off = htons(0);
	ip_header->ip_ttl = 255;
	ip_header->ip_p = IPPROTO_UDP;
	ip_header->ip_src = src_ip_v;
	ip_header->ip_dst = dst_ip_v;
	ip_header->ip_sum = 0;  //set by protocol stack 

    /* Index of the network device */
    socket_address.sll_ifindex = if_idx.ifr_ifindex;
    /* Address length*/
    socket_address.sll_halen = ETH_ALEN;
    /* Destination MAC */
    socket_address.sll_addr[0] = dst_mac[0];
    socket_address.sll_addr[1] = dst_mac[1];
    socket_address.sll_addr[2] = dst_mac[2];
    socket_address.sll_addr[3] = dst_mac[3];
    socket_address.sll_addr[4] = dst_mac[4];
    socket_address.sll_addr[5] = dst_mac[5];

    /* Send packet */
    if (sendto(sockfd, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0) {
        printf("Send failed\n");
	}

	close(sockfd);

    return 0;
}
