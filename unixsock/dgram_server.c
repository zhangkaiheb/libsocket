#include <sys/types.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>

#include "sock_comm.h"


int main(int argc, char *argv[])
{
	int buflen = 0;
	int sock = 0;
	char buf[4096];
	int addrlen = 0;
	socklen_t fromlen;
	struct sockaddr_un addr, from;
	char *path = DGRAM_SOCK_PATH;

	sock = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("socket\n");
		return -1;
	}
	unlink(path);

	bzero(&addr, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);
	addrlen = sizeof(addr.sun_family) + strlen(addr.sun_path);

	if (bind(sock, (struct sockaddr *) &addr, addrlen) < 0) {
		perror("bind failed!");
		goto out;
	}

	for (;;) {
		buflen = recvfrom(sock, buf, sizeof(buf), 0,
				(struct sockaddr *)&from, &fromlen);
		if (buflen < 1) {
			continue;
		}

		fprintf(stdout, "Received message length %d: %s\n", buflen, buf);
		//if (sendto(sock, buf, buflen, 0 /*MSG_NOSIGNAL */,
		//			(struct sockaddr *)&from, fromlen) < 0) {

		//	perror("sendto");
		//	usleep(500);
		//	/* try again. */
		//	sendto(sock, buf, buflen, MSG_NOSIGNAL,
		//			(struct sockaddr *)&from, fromlen);
		//}
	}

out:
	close(sock);
	return -1;
}


