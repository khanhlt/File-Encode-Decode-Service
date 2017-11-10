/* getfile server that handles only one client at a time */
#include <stdio.h> /* printf and standard I/O */
#include <sys/socket.h> /* socket, connect, socklen_t */
#include <arpa/inet.h> /* sockaddr_in, inet_pton */
#include <string.h> /* strlen */
#include <stdlib.h> /* atoi, EXIT_FAILURE */
#include <fcntl.h> /* 0_WRONLY, O_RDONLY */
#include <unistd.h> /* close, write, read */

#define SRV_PORT 1610 /* default port number */
#define MAX_RECV_BUF 256
#define MAX_SEND_BUF 256

int send_file(int , char*);

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

int send_file (int sock, char* file_name) {
	int sent_count; /* how many sending chunks, for debugging */
	ssize_t read_bytes, /* bytes read from local file */
	sent_bytes,	/* bytes sent to connected socket */
	sent_file_size;
	char send_buf[MAX_SEND_BUF]; /* max chunk size for sending file */
	char *errmsg_notfound = "File not found\n";
	int f; /* file handle for reading local file */

	sent_count = 0;
	sent_file_size = 0;

	/* attempt to open requested file for reading */
	if ((f = open(file_name, O_RDONLY)) < 0) /* can't open requested file */
	{
		perror(file_name);
		if ((sent_bytes = send(sock, errmsg_notfound, strlen(errmsg_notfound), 0)) < 0) {
			perror("send error");
			return -1;
		}
	}
	else /*open file successful */
	{
		printf("Sending file: %s\n", file_name);
		while ((read_bytes = read(f, send_buf, MAX_RECV_BUF)) > 0) {
			if((sent_bytes = send(sock, send_buf, read_bytes, 0)) < read_bytes) {
				perror("send error");
				return -1;
			}
			sent_count++;
			sent_file_size += sent_bytes;
		}
		close(f);
	}

	printf("Send done! Sent %d bytes in %d send(s)\n\n", sent_file_size, sent_count);
	return sent_count;
}