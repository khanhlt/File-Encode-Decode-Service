/*
	created by khanhlt - 13/11/2017
	=====
	server recv ==> decode/encode ==> send file back to client
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
#include "base64.h"

#define PORT 1610
#define MAX_RECV_BUF 256
#define LISTEN_ENQ 5

int main (int argc, char* argv[]) {
	struct sockaddr_in srv_addr;	/* socket parameter for bind */
	struct sockaddr_in cli_addr;
	int listen_fd;
	int conn_fd;
	FILE* fd;
	int cli_len;

	/* create socket */
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("socket error");
		exit(EXIT_FAILURE);
	}		

	/* fill in socket structure */
	memset(&srv_addr, 0, sizeof(srv_addr));
	memset(&cli_addr, 0, sizeof(cli_addr));

	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	/* if port number supplied, use it, otherwise use PORT */
	srv_addr.sin_port = (argc > 1) ? htons(atoi(argv[1])) : htons(PORT);

	/* bind socket to the port */
	if (bind(listen_fd, (struct sockaddr*) &srv_addr, sizeof(srv_addr)) < 0) {
		perror("bind error");
		exit(EXIT_FAILURE);
	}

	/* listen for clients on the socket */
	if (listen(listen_fd, LISTEN_ENQ) < 0) {
		perror("listen error");
		exit(EXIT_FAILURE);
	}

	for ( ; ; ) {
		printf ("Waiting for a client to connect...\n");
		cli_len = sizeof(cli_addr);
		if ((conn_fd = accept(listen_fd, (struct sockaddr*) &cli_addr, &cli_len)) < 0) {
			perror("accept error");
			break;
		} 

		printf("client connected\n");

		/* receive encode/decode signal */
		char cli_msg[MAX_RECV_BUF] = "";
		if (recv(conn_fd, cli_msg, MAX_RECV_BUF, 0) < 0) {
			printf("unable to receive encode/decode signal\n");
			close(conn_fd);
		}
		printf("%s\n", cli_msg);

		char* reply = "received signal OK!";
		if (send(conn_fd, reply, sizeof(reply), 0) < 0) {
			printf("unable to reply\n");
			close(conn_fd);
		}

		if ((fd = fopen("srv_result.txt", "w")) == NULL) {
			perror("error creating file");
			return -1;
		}

		/* receive file from client */
		int len;
		char buffer[MAX_RECV_BUF + 1];
		while ((len = recv(conn_fd, buffer, MAX_RECV_BUF, 0)) > 0) {
			fwrite(buffer, 1, len, fd);
		}
		fclose(fd);

		/* encode/decode */
		if (strcmp(cli_msg,"en") == 0)
			encode("srv_result.txt");
		else if (strcmp(cli_msg, "de") == 0)
			decode("srv_result.txt");
		else {
			printf("wrong encode/decode signal\n");
			close(conn_fd);
		}

		/* send file back to client */
		int fd_ = open("final_result.txt", O_RDONLY);
		struct stat stat_buf;
		off_t offset = 0;
		if (fd_ < 0) {
			printf ("unable to open file\n");
		}

		/*get the size of the file to be sent */
		fstat(fd_, &stat_buf);
		if ((sendfile(conn_fd, fd_, &offset, stat_buf.st_size)) == -1) {
			printf("error from sendfile \n");
			exit(1);
		}
		close(fd_);

		// printf("Closing connection\n");
		close(conn_fd);
	}
	close(listen_fd);
	return 0;
}