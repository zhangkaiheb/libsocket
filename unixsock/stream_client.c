#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>

#include "sock_comm.h"


int main(int argc, char *argv[])
{
	int sock;
	struct timeval tv;
	char rbuf[1024] = {0};
	struct sockaddr_un addr;
	char *path = STREAM_SOCK_PATH;
	char sbuf[] = "stream unix sock test";

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock == -1) {
		perror("Failed create socket\n");
		return -1;
	}

	tv.tv_sec = 5;
	tv.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	bzero(&addr, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);

	if (connect(sock, (struct sockaddr *)&addr,
				sizeof(struct sockaddr_un)) == -1) {
		perror("Connect\n");
		goto out;
	}

	if (send(sock, sbuf, sizeof(sbuf), 0) == -1) {
		perror("Send\n");
		goto out;
	}

	if (recv(sock, rbuf, sizeof(rbuf), 0) == -1) {
		perror("Recv\n");
		goto out;
	}

	printf("Reply message: %s\n", rbuf);

	close(sock);
	return 0;
out:
	close(sock);
	return -1;	
}

