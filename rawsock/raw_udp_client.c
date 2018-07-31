#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "raw_comm.h"


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

static int init_socket()
{  
	int fd = -1;
	int on = 1;  

	if ((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1) {  
	//if ((fd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {  
		perror("socket");
		return -1;  
	}
	if (setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) == -1) {  
		perror("setsockopt");
		close(fd);  
		return -1;  
	}	

	return fd;
}

char *construct_pkt(const void *msg, size_t len,
			uint32_t src_ip, uint16_t src_port,
			uint32_t dst_ip, uint16_t dst_port, int *pktlen)  
{  
	static char buf[1024]; 
	ip_hdr *ip_header;
	udp_hdr *udp_header;
	psd_hdr *pseudo_header;
	char *data;
	uint16_t udp_len;
	uint16_t total_len;
	size_t udp_check_len;
	
	if (!msg || len <= 0)
		return NULL;  

	ip_header = (ip_hdr *)buf;  
	udp_header = (udp_hdr *)(ip_header + 1);  
	data = (char *)(udp_header + 1); 
	pseudo_header = (psd_hdr *)((char *)udp_header - sizeof(psd_hdr));  
	if (sizeof(*ip_header) + sizeof(*udp_header) +
			len + len % 2 > sizeof(buf)) {  
		goto out;  
	} 

	udp_len = sizeof(*udp_header) + len;  
	total_len = sizeof(*ip_header) + sizeof(*udp_header) + len;  

	pseudo_header->src_ip = src_ip;  
	pseudo_header->dst_ip = dst_ip;  
	pseudo_header->zero = 0;  
	pseudo_header->protocol = IPPROTO_UDP;  
	pseudo_header->udp_len = htons(udp_len);  

	udp_header->source = htons(src_port);  
	udp_header->dest = htons(dst_port);  
	udp_header->len = htons(sizeof(*udp_header) + len);  
	udp_header->check = 0;  

	memcpy(data, msg, len);  
	udp_check_len = sizeof(*pseudo_header) + sizeof(*udp_header) + len;  
	if (len % 2 != 0) {  
		udp_check_len += 1; 
		*(data + len) = 0;  
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
	ip_header->ip_src = src_ip;  
	ip_header->ip_dst = dst_ip;  
	ip_header->ip_sum = 0;  //set by protocol stack     
	//ip_header->ip_sum = check_sum(ip_header, sizeof(*ip_header));    

	if (pktlen)
		*pktlen = total_len;

	return buf;
out:
	return NULL;
}

static void send_loop(uint32_t dst_ip, uint16_t port, uint32_t src_ip)
{
	int fd;
	char *pkt;
	int pktlen;
	char buf[2048];
	struct sockaddr_in dst_addr; 

	fd = init_socket();
	if (fd == -1)
		exit(-1);

	bzero(&dst_addr, sizeof(dst_addr));
	dst_addr.sin_family = AF_INET;  
	dst_addr.sin_addr.s_addr = dst_ip;  
	dst_addr.sin_port = htons(port);  

	while (1) {
		bzero(buf, sizeof(buf));

		if (read(STDIN_FILENO, buf, sizeof(buf)) == -1)
			continue;

		pkt = construct_pkt(buf, strlen(buf)+1,
					src_ip, port, dst_ip, port, &pktlen); 
		if (pkt == NULL)
			continue;

		if (sendto(fd, pkt, pktlen, 0,
					(struct sockaddr *)&dst_addr, sizeof(dst_addr)) != pktlen) {  
			perror("sendto:");
			break;
		}
	}

	close(fd);
}

static void usage(void)
{
	printf("raw_udpc dst_ip dst_port src_ip\n");
}

int main( int argc, char **argv)
{
	uint16_t port = 5000;
	uint32_t src_ip = 0; 
	uint32_t dst_ip = 0; 

	if (argc < 4) {
		usage();
		return -1;
	}
	port = atoi(argv[2]);

	if (!(inet_pton(AF_INET, argv[3], &src_ip)) ||
		!(inet_pton(AF_INET, argv[1], &dst_ip))) {  
		usage();
		return -1;  
	} 

	send_loop(dst_ip, port, src_ip);

	return 0;
}

