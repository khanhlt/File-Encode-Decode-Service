/*
	created by khanhlt - 13/11/2017
	=====
	client send - receive file
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>

#define PORT 1610
#define MAX_RECV_BUF 256

int main (int argc, char* argv[]) {
	int cli_sock;
	struct sockaddr_in srv_addr;
	struct stat stat_buf;
	off_t offset = 0;

	if (argc < 4) {
		printf("usage: %s <filename> <en/de> <IP address> [port number]\n", argv[0]);
		printf("en: stands for encode\nde: stands for decode\n");
		exit(EXIT_FAILURE);
	}

	/* zero - fill srv_addr */
	memset(&srv_addr, 0, sizeof(srv_addr));

	/* create client socket */
	cli_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	/* construct srv_addr struct */
	srv_addr.sin_family = AF_INET;
	if (inet_pton(AF_INET, argv[3], &(srv_addr.sin_addr)) < 1) {
		printf("Invalid IP address\n");
		exit(EXIT_FAILURE);
	}

	/* if port number supplied, use it, otherwise use PORT */
	srv_addr.sin_port = (argc > 4) ? htons(atoi(argv[4])) : htons(PORT);

	if (connect(cli_sock, (struct sockaddr*) &srv_addr, sizeof(srv_addr)) < 0) {
		perror("connect error");
		exit(EXIT_FAILURE);
	}

	printf ("connected to : %s:%d ...\n", argv[3], PORT);

	// /* send encode/decode signal */
	if (send(cli_sock, argv[2], strlen(argv[2]), 0) < 0) {
		printf("error sending encode/decode signal\n");
		exit(1);
	}

	/* receive reply from server */
	char server_reply[MAX_RECV_BUF];
	if (recv(cli_sock, server_reply, MAX_RECV_BUF, 0) < 0) {
		printf("server don't receive encode/decode signal\n");
		exit(1);
	}


	/* open the file to be sent */
	char* file_name = argv[1];
	int fd = open(file_name, O_RDONLY);
	if (fd < 0) {
		printf("unable to open '%s'\n", file_name);
	}

	/* get the size of the file to be sent */
	fstat(fd, &stat_buf);

	/* copy file using sendfile() */
	offset = 0;
	if ((sendfile(cli_sock, fd, &offset, stat_buf.st_size)) == -1) {
		printf("error from sendfile \n");
		exit(1);
	}

	/* close file */
	close(fd);

	/* finish sending file */
	shutdown(cli_sock, SHUT_WR);

	/* recv file from server */
	FILE *fd_;
	if ((fd_ = fopen("cli_result.txt", "w")) == NULL) {
			perror("error creating file");
			return -1;
		}

	int len;
	char buffer[MAX_RECV_BUF + 1];
	while ((len = recv(cli_sock, buffer, MAX_RECV_BUF, 0)) > 0) {
		fwrite(buffer, 1, len, fd_);
	}
	fclose(fd_);

	/* close socket */
	close(cli_sock);
	return 0;
}