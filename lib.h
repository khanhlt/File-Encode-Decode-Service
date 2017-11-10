/*
	created by khanhlt - 11/11/2017
*/
#include <stdio.h> /* printf and standard i/o */
#include <sys/socket.h> /* socket, bind, listen, accept, socklen_t */
#include <arpa/inet.h> /* sockaddr_in, inet_ntop */
#include <string.h> /* strlen */
#include <stdlib.h> /* atoi, EXIT_FAILURE */
#include <fcntl.h> /* open, O_RDONLY */
#include <unistd.h> /* close, read */
#define SRV_PORT 1610 /* default port number */
#define LISTEN_ENQ 5 /* for listen backlog */
#define MAX_RECV_BUF 256
#define MAX_SEND_BUF 256

int send_file(int, char*);
int recv_file(int);


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

int recv_file(int sock) {
	ssize_t sent_bytes, rcvd_bytes, rcvd_file_size;
	int recv_count; /* count of recv() calls */
	char recv_str[MAX_RECV_BUF]; /* buffer to hold received data */

	int f_result;

	/* create result file */
	if ((f_result = open("result.txt", O_WRONLY|O_CREAT, 0644)) < 0) {
		perror("error creating file");
		return -1;
	}

	recv_count = 0; /* number of recv() calls required to receive the file */
	rcvd_file_size = 0; /* size of received file */

	/* continue receiving until ? (data or close) */
	while ((rcvd_bytes = recv(sock, recv_str, MAX_RECV_BUF, 0)) > 0) {
		recv_count ++;
		rcvd_file_size += rcvd_bytes;

		if(write(f_result, recv_str, rcvd_bytes) < 0) {
			perror("error writing to file");
			return -1;
		}
	}
	close(f_result);
	printf("Received: %d bytes in %d recv(s)\n", rcvd_file_size, recv_count);
	return rcvd_file_size;
}
