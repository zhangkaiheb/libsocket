#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <error.h>
#include <errno.h>


struct rtnlsock
{
	int sock;
	char *name;
	struct sockaddr_nl snl;
} rtnetlink = {-1, "rtnetlink", {0}};

static struct nl_message {
	int key;
	char *str;
} nlmsg_str[] =
{
	{RTM_NEWROUTE, "RTM_NEWROUTE"},
	{RTM_DELROUTE, "RTM_DELROUTE"},
	{RTM_GETROUTE, "RTM_GETROUTE"},
	{RTM_NEWLINK,  "RTM_NEWLINK"},
	{RTM_DELLINK,  "RTM_DELLINK"},
	{RTM_GETLINK,  "RTM_GETLINK"},
	{RTM_NEWADDR,  "RTM_NEWADDR"},
	{RTM_DELADDR,  "RTM_DELADDR"},
	{RTM_GETADDR,  "RTM_GETADDR"},
	{0,            NULL}
};

static char *
lookup(struct nl_message *mes, int key)
{
	struct nl_message *p;

	for (p = mes; p->key != 0; p++)
	{
		if (p->key == key)
			return p->str;
	}
	return "";
}

static int
rtnl_socket(struct rtnlsock *nl, unsigned long groups, uint8_t non_block)
{
	int ret;
	struct sockaddr_nl snl;
	int sock;
	int namelen;
	int bufsize = 4194303; /* 4M */

	sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (sock < 0) {
		perror("Can't open socket:");
		return -1;
	}

	if (non_block) {
		ret = fcntl(sock, F_SETFL, O_NONBLOCK);
		if (ret < 0) {
			perror("Can't set socket flags:");
			close(sock);
			return -1;
		}
	}

	/* correct buffer size */
	setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&bufsize,
			(int)sizeof(bufsize));
	setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&bufsize,
			(int)sizeof(bufsize));

	bzero(&snl, sizeof(snl));
	snl.nl_family = AF_NETLINK;
	snl.nl_groups = groups;

	ret = bind(sock, (struct sockaddr *)&snl, sizeof(snl));
	if (ret < 0) {
		fprintf(stderr, "Can't bind %s socket to group 0x%x: %s",
				nl->name, snl.nl_groups, strerror(errno));
		close(sock);
		return -1;
	}

	/* multiple netlink sockets will have different nl_pid */
	namelen = sizeof(snl);
	ret = getsockname(sock, (struct sockaddr *)&snl, (socklen_t *)&namelen);
	if (ret < 0 || namelen != sizeof(snl)) {
		perror("Can't get socket name:");
		close(sock);
		return -1;
	}

	nl->snl = snl;
	nl->sock = sock;
	return ret;
}

static int
rtnetlink_parse_info(struct rtnlsock *nl)
{
	int status;
	int ret = 0;

	while (1) {
		char buf[81920];
		struct iovec iov = {buf, sizeof(buf)};
		struct sockaddr_nl snl;
		struct msghdr msg = {(void*)&snl, sizeof(snl), &iov, 1, NULL, 0, 0};
		struct nlmsghdr *h;

		status = recvmsg(nl->sock, &msg, 0);
		if (status < 0) {
			if (errno == EINTR)
				continue;
			if (errno == EWOULDBLOCK)
				break;

			perror("recvmsg overrun:");
			continue;
		}

		if (snl.nl_pid != 0) {
			fprintf(stdout, "Ignoring non kernel message from pid %u\n",
					snl.nl_pid);
			continue;
		}

		if (status == 0) {
			fprintf(stdout, "Netlink EOF");
			return -1;
		}

		if (msg.msg_namelen != sizeof(snl)) {
			fprintf(stderr, "sender address length error: length %d\n",
					msg.msg_namelen);
			return -1;
		}

		for (h = (struct nlmsghdr *)buf; NLMSG_OK(h, status);
				h = NLMSG_NEXT(h, status))
		{
			/* Finish of reading. */
			if (h->nlmsg_type == NLMSG_DONE) {
				return ret;
			}

			/* OK we got netlink message. */
			fprintf(stdout, "rtnetlink type %s(%u), seq=%u, pid=%d\n",
					lookup(nlmsg_str, h->nlmsg_type),
					h->nlmsg_type, h->nlmsg_seq, h->nlmsg_pid);
		}
	}
	return ret;
}

int main(int arg, char * args[])
{
	int ret;
	unsigned long groups;

	groups = RTMGRP_LINK | 
		RTMGRP_IPV4_ROUTE  | 
		RTMGRP_IPV4_IFADDR |
		RTMGRP_IPV6_ROUTE  |
		RTMGRP_IPV6_IFADDR;

	ret = rtnl_socket(&rtnetlink, groups, 1);
	if (ret == -1)
		return ret;

	while (1) {
		rtnetlink_parse_info(&rtnetlink);
	}

	close(rtnetlink.sock);

	return 0;
}

