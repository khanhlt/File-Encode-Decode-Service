/*
	created by khanhlt - 10/11/2017
 */
#include "lib.h"

int main(int argc, char* argv[]) {
	int listen_fd, conn_fd;
	struct sockaddr_in srv_addr, cli_addr;
	socklen_t cli_len;
	char print_addr[INET_ADDRSTRLEN]; /* readable IP address */

	memset(&srv_addr, 0, sizeof(srv_addr)); /* zero-fill srv_addr structure */
	memset(&cli_addr, 0, sizeof(cli_addr)); /* zero-fill cli_addr structure */
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	/* if port number supplied, use it, otherwise use SRV_PORT */
	srv_addr.sin_port = (argc > 1) ? htons(atoi(argv[1])) : htons(SRV_PORT);

	if ((listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("socket error");
		exit(EXIT_FAILURE);
	}

	/* bind to creat socket */
	if (bind(listen_fd, (struct sockaddr*) &srv_addr, sizeof(srv_addr)) < 0) {
		perror("bind error");
		exit(EXIT_FAILURE);
	}

	printf("Listening on port number %d ...\n", ntohs(srv_addr.sin_port));
	if (listen(listen_fd, LISTEN_ENQ) < 0) {
		perror("listend error");
		exit(EXIT_FAILURE);
	}

	for ( ; ; ) {
		cli_len = sizeof(cli_addr);
		printf("Waiting for a client to connect...\n\n");

		/* block until some client connects */
		if ((conn_fd = accept(listen_fd, (struct sockaddr*) &cli_addr, &cli_len)) < 0) {
			perror("accept error");
			break;
		}

		/* convert numeric IP to readable format for displaying */
		inet_ntop(AF_INET, &(cli_addr.sin_addr), print_addr, INET_ADDRSTRLEN);
		printf("Client connected from %s:%d\n", print_addr, ntohs(cli_addr.sin_port));

		recv_file(conn_fd); 

		printf("Closing connection\n");
		close(conn_fd); /* close connected socket */
	}
	close(listen_fd);
	return 0;
}