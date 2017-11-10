/*
	created by khanhlt - 10/11/2017
*/
#include "lib.h"

int main(int argc, char* argv[]) {
	int sock_fd;
	struct sockaddr_in srv_addr;

	if (argc < 3) {
		printf("usage: %s <filename> <IP address> [port number]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	memset(&srv_addr, 0, sizeof(srv_addr)); /* zero-fill srv_addr structure */

	/* create a client socket */
	sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	srv_addr.sin_family = AF_INET; /* internet address family */

	/* convert command line argument to numeric IP */
	if (inet_pton(AF_INET, argv[2], &(srv_addr.sin_addr)) < 1) {
		printf ("Invalid IP address\n");
		exit(EXIT_FAILURE);
	}

	/* if port number supplied, use it, otherwise use SRV_PORT */
	srv_addr.sin_port = (argc > 3) ? htons(atoi(argv[3])):htons(SRV_PORT);

	if (connect(sock_fd, (struct sockaddr*) &srv_addr, sizeof(srv_addr)) < 0) {
		perror("connect error");
		exit(EXIT_FAILURE);
	}

	printf("connected to:%s:%d ...\n", argv[2], SRV_PORT);

	send_file(sock_fd, argv[1]); /* argv[1] = file name */

	/* close socket */
	if (close(sock_fd) < 0) {
		perror("socket close error");
		exit(EXIT_FAILURE);
	}
	return 0;
}