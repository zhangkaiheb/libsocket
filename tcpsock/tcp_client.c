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
	int port, sock;
    char buf[2048] = {0};
    struct sockaddr_in addr;

    if (arg < 3) {
        printf("please input: ip port!\n");
        return -1;
    }
    port = atoi(args[2]);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("create socket failed !\n");
        return -1;
    }
    /*
    int on = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) == -1) {
        printf("setsockopt failed ! error message :%s\n", strerror(errno));
        return -1;
    }
    */
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(args[1]);

	if (connect(sock,(struct sockaddr *)&addr, sizeof(addr)) == -1) {
		perror("connect\n");
		goto out;
	}

    while (1) {
        bzero(buf, sizeof(buf));
        //read
        if (read(STDIN_FILENO, buf, sizeof(buf)) == -1)
			continue;

        if (send(sock, buf, strlen(buf), 0) == -1) {
            perror("sendto failed!\n");
            break;
        }

		if (recv(sock, buf, sizeof(buf), 0) == -1) {
            perror("recvfrom failed!\n");
			break;
		} else {
			printf("received (%s)\n", buf);
		}
    }
    close(sock);
    return 0;
out:
    close(sock);
	return -1;
}

