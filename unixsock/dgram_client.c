#include <ctype.h>
#include <sys/types.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "sock_comm.h"

int main(int argc, char *argv[])
{
	int sock;
	struct timeval tv_to;
    //char buf[4096] = {0};
	struct sockaddr_un addr;
    socklen_t addrlen;
	char *path = DGRAM_SOCK_PATH;
	//struct sockaddr_un from;
    char sbuf[] = "unix dgram socket test";

	sock = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("Failed create socket\n");
		return -1;
	}

	tv_to.tv_sec = 5;
	tv_to.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv_to, sizeof(tv_to));

	bzero(&addr, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);
	addrlen = sizeof(addr.sun_family) + strlen(addr.sun_path);

	if (sendto(sock, sbuf, sizeof(sbuf), 0,
			(struct sockaddr *)&addr, addrlen) == -1) {
		perror("sendto failed!");
		goto out;
	}

	//if (recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&from, &flen) == -1) {
	//	perror("recvfrom failed!\n");
	//	goto error;
	//}
	//fprintf(stdout, "Reply message: %s\n", buf);

	close(sock);
	return 0;
out:
	close(sock);
	return -1;
}


