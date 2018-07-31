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


uint16_t port = 5000;
uint32_t ipaddr;

static int init_sock()  
{
	int fd = -1;

	if ((fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
		perror("socket");
		return -1;
	}

	return fd;  
}

static int filter_pkt(char *pkt, int len)
{
	ip_hdr *iphdr;
	udp_hdr *udphdr;
	int ipoff = sizeof(struct ethhdr);
	int udpoff = ipoff + sizeof(ip_hdr);
	int dataoff;

	if (len < udpoff + sizeof(udp_hdr))
		return -1;

	iphdr = (ip_hdr *)(pkt + ipoff);
	udphdr = (udp_hdr *)(pkt + udpoff);

	if (iphdr->ip_src != ipaddr)
		return -1;

	if (udphdr->dest != htons(port))
		return -1;

	if ((iphdr->ip_p == 6/*TCP*/) &&
			(len > udpoff + sizeof(tcp_hdr))) {
		dataoff = udpoff + sizeof(tcp_hdr);
	}
	else if (iphdr->ip_p == 17/*UDP*/)
		dataoff = udpoff + sizeof(udp_hdr);
	else
		return -1;

	printf("Received: %s\n", pkt + dataoff);
	return 0;
}

static int recv_loop(int fd)  
{
	int ret;
	char rbuf[2048];
	struct sockaddr_in from;
	socklen_t fromlen = sizeof(from);

	while (1) {
		bzero(rbuf, sizeof(rbuf));
		bzero(&from, sizeof(from));

		ret = recvfrom(fd, rbuf, sizeof(rbuf) - 1, 0,
				(struct sockaddr *)&from, &fromlen);
		if (ret <= 0)
			continue;

		filter_pkt(rbuf, ret);
	}

	close(fd);
	return 0;
} 

static void usage()
{
	printf("Usage: \nraws -d filtered_src_ip -p [filtered_dst_port]\n");
	printf("\t[filtered_dest_port]    (default 5000) \n");
}

static int param_check(int argc, char **argv)
{
	int c;
	char ip[64] = {0};

	while ((c = getopt(argc, argv, "p:d:")) != -1) {
		if (c == 'p') {
			port = atoi(optarg);
		}
		else if (c == 'd') {
			strncpy(ip, optarg, sizeof(ip)-1);
		} else {
			usage();
			return -1;
		}
	}
	if (!ip[0]) {
		usage();
		return -1;
	}
	if (!inet_pton(AF_INET, ip, &ipaddr))
		return -1;

	return 0;
}

int main(int argc, char **argv)
{
	int fd = -1;

	if (param_check(argc, argv))
		return -1;

	fd = init_sock();
	if (fd < 0)
		return -1;

	recv_loop(fd);

	return 0;
}

