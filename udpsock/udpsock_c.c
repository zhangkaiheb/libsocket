#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int arg, char * args[])
{
	socklen_t addrlen;
	char buf[2048] = {0};
	struct sockaddr_in addr;

	if (arg < 3) {
		printf("please input: ip port!\n");
		return -1;
	}
	int port = atoi(args[2]);
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd == -1) {
		printf("create socket failed ! error message :%s\n", strerror(errno));
		return -1;
	}
	/*
	   int on = 1;
	   if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) == -1) {
	   printf("setsockopt failed ! error message :%s\n", strerror(errno));
	   return -1;
	   }
	 */
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(args[1]);

	while (1) {
		bzero(buf, sizeof(buf));
		//read
		if (read(STDIN_FILENO, buf, sizeof(buf)) == -1)
			continue;

		if (sendto(fd, buf, strlen(buf), 0, (struct sockaddr *) &addr,
					sizeof(addr)) == -1) {
			printf("sendto failed! %s\n", strerror(errno));
			break;
		}

		if (recvfrom(fd, buf, sizeof(buf), 0,
					(struct sockaddr *)&addr, &addrlen) == -1) {
			printf("recvfrom failed! :%s\n", strerror(errno));
			break;
		} else {
			printf("received (%s)\n", buf);
		}
	}
	close(fd);

	return 0;
}

