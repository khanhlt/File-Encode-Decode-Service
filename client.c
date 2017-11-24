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
	char file_path[MAX_RECV_BUF];
	char en_or_de[4];

	if (argc < 2) {
		printf("usage: %s <IP address> [port number]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* zero - fill srv_addr */
	memset(&srv_addr, 0, sizeof(srv_addr));

	/* create client socket */
	cli_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	/* construct srv_addr struct */
	srv_addr.sin_family = AF_INET;
	if (inet_pton(AF_INET, argv[1], &(srv_addr.sin_addr)) < 1) {
		printf("Invalid IP address\n");
		exit(EXIT_FAILURE);
	}

	/* if port number supplied, use it, otherwise use PORT */
	srv_addr.sin_port = (argc > 2) ? htons(atoi(argv[2])) : htons(PORT);

	if (connect(cli_sock, (struct sockaddr*) &srv_addr, sizeof(srv_addr)) < 0) {
		perror("connect error");
		exit(EXIT_FAILURE);
	}

	printf ("connected to : %s:%d ...\n", argv[1], PORT);

	// /* send encode/decode signal */
	printf("Moi ban nhap duong dan toi file: ");
	scanf("%s", file_path);
	printf("Ban muon encode hay decode? Nhap 'en' for encode, 'de' for decode: ");
	scanf("%s", en_or_de);
	if (send(cli_sock, en_or_de, MAX_RECV_BUF, 0) < 0) {
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
	int fd = open(file_path, O_RDONLY);
	if (fd < 0) {
		printf("unable to open '%s'\n", file_path);
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