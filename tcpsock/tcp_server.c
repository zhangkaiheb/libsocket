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
	int port;
	int sock, csock;
    char buf[2048]={0};
	const char result[] = "success";
    struct sockaddr_in addr;
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    if (arg < 2) {
        printf("please input: port!\n");
        return -1;
    }
    port = atoi(args[1]);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("open socket failed !\n");
        return -1;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //bind port
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("Bind IP failed !\n");
        goto out;
    }

	/* listen */
	if (listen(sock, 10) == -1) {
		perror("Server-listen()\n");
		goto out;
	}

	addrlen = sizeof(client_addr);
	bzero(&client_addr, sizeof(client_addr));

	if ((csock = accept(sock, (struct sockaddr *)&client_addr, &addrlen)) == -1) {
		perror("Server-accept()\n");
		goto out;
	}

	while (1) {
		bzero(buf, sizeof(buf));		

		if (recv(csock, buf, sizeof(buf), 0) == -1) {
			perror("recvfrom failed !\n");
		} else {
			printf("from %s:%s\n", inet_ntoa(client_addr.sin_addr), buf);

			if (send(csock, result, sizeof(result), 0) == -1) {
				perror("sendto failed !\n");
			}
		}
	}
	close(csock);
	close(sock);
	return 0;

out:
	close(sock);
    return -1;
}


