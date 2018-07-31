#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>

#include "sock_comm.h"


int main(int argc, char *argv[])
{
	int len, sock, csk;
	char buf[2048];
	socklen_t fromlen;
	struct sockaddr_un addr, from;
	char *path = STREAM_SOCK_PATH;

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock == -1) {
		perror("Socket create failed\n");
		return -1;
	}
	unlink(path);

	bzero(&addr, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);

	if (bind(sock, (struct sockaddr *)&addr,
				sizeof(struct sockaddr_un)) == -1) {
		perror("Bind\n");
		goto out;
	}

	if (listen(sock, 5) == -1) {
		perror("Listen");
		goto out;
	}

	for (;;) {
		csk = accept(sock, (struct sockaddr *)&from, &fromlen);  
		if (csk == -1) {
			perror("Accept");
			continue;
		}

		if ((len = recv(csk, buf, sizeof(buf), 0)) == -1) {
			perror("Recv");
			close(csk);
			goto out;
		}
		printf("Message received length %d: %s\n", len, buf); 

		if (send(csk, buf, sizeof(buf), MSG_NOSIGNAL) == -1) {
			perror("Send\n");
		}
		close(csk);
	}

out:
	close(sock);
	unlink(path);

	return 0;
}

