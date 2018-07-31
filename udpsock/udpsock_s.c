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
	int ret;
	char buf[2048]={0};
	struct sockaddr_in addr;
	struct sockaddr_in from;
	socklen_t fromlen = sizeof(from);
	const char result[] = "success";

	if (arg < 2) {
		printf("please input: port!\n");
		return -1;
	}

	int port = atoi(args[1]);
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd == -1) {
		printf("open socket failed ! %s\n", strerror(errno));
		return -1;
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//bind port
	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		printf("bind IP failed ! %s\n", strerror(errno));
		goto out;
	}

	while(1) {
		bzero(buf, sizeof(buf));		
		bzero(&from, sizeof(from));

		if ((ret = recvfrom(fd, buf, sizeof(buf), 0,
					(struct sockaddr *)&from, &fromlen)) == -1) {
			printf("recvfrom failed ! %s\n", strerror(errno));
			goto out;
		} else {
			printf("Recv %d from %s:%s\n", ret, inet_ntoa(from.sin_addr), buf);

			if (sendto(fd, result, sizeof(result), 0,
					(struct sockaddr *)&from, fromlen) == -1) {
				printf("sendto failed ! %s\n", strerror(errno));
				goto out;
			}
		}
	}

out:
	close(fd);

	return -1;
}


