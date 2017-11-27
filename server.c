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
#include <signal.h>
#include <sys/wait.h>
#include "base64.h"

#define PORT 1610
#define MAX_RECV_BUF 256
#define LISTEN_ENQ 5

void sig_chld(int);

int main (int argc, char* argv[]) {
	struct sockaddr_in srv_addr;	/* socket parameter for bind */
	struct sockaddr_in cli_addr;
	int listen_fd;
	int conn_fd;
    pid_t child_pid;    /* pid of child process */
	FILE* fd;
	int cli_len;
    char print_addr[INET_ADDRSTRLEN];

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
    printf("Listening on port number %d ...\n", ntohs(srv_addr.sin_port));
	if (listen(listen_fd, LISTEN_ENQ) < 0) {
		perror("listen error");
		exit(EXIT_FAILURE);
	}
    
    signal (SIGCHLD, sig_chld);

	for ( ; ; ) {
		printf ("Waiting for a client to connect...\n");
		cli_len = sizeof(cli_addr);
		if ((conn_fd = accept(listen_fd, (struct sockaddr*) &cli_addr, &cli_len)) < 0) {
			perror("accept error");
			break;
		} 
        
        inet_ntop(AF_INET, &(cli_addr.sin_addr), print_addr, INET_ADDRSTRLEN);
		printf("client connected from %s:%d\n", print_addr, ntohs(cli_addr.sin_port));
        
        /* fork a new child process */
        if ((child_pid = fork()) == 0) /* fork returns 0 for child */
        {
            close(listen_fd); /* close child's copy of listen_fd */
            /* receive encode/decode signal */
	    	char cli_msg[MAX_RECV_BUF] = "";
	    	if (recv(conn_fd, cli_msg, MAX_RECV_BUF, 0) < 0) {
	    		printf("unable to receive encode/decode signal\n");
	    		close(conn_fd);
	    	}

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
            printf("Received file from client\n");
	    	fclose(fd);

	    	/* encode/decode */
	    	if (strcmp(cli_msg,"en") == 0) {
	    		encode("srv_result.txt");
                printf("encoded\n");
            }
	    	else if (strcmp(cli_msg, "de") == 0) {
	    		decode("srv_result.txt");
                printf("decoded\n");
            }
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
            printf("Returned to file to client\n");
    		close(fd_);
            
            printf("Closing connection\n");
            close(conn_fd);
            exit(0);  
        }
				// printf("Closing connection\n");
		close(conn_fd);
	}
	close(listen_fd);
	return 0;
}

void sig_chld(int signo) 
{
    pid_t pid;
    int stat;
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("child %d terminated\n", pid);
    return;
}
