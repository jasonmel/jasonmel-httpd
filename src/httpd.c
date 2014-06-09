#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "httpd.h"

#define MAX_NUM 1024

extern char **environ;

void errexit(const char *, ...);

int cgi_handler(char *req)
{
	char reqFile[MAX_NUM];
	char qryStr[MAX_NUM];
	char tmp[MAX_NUM];
	char *argv[2];
	char *token;

	strcpy(tmp, req);
	token = strtok(tmp, "?");
	strcpy(reqFile, token);
	token = strtok(NULL, "?");
	if (token != NULL) strcpy(qryStr, token);
	else strcpy(qryStr, "");

	argv[0] = (char *) calloc(strlen(req) + 1, sizeof(char));
	strcpy(argv[0], reqFile);
	argv[1] = (char *) 0;

	setenv("PATH", ".", 1);
	setenv("QUERY_STRING", qryStr, 1);

	//if (execvp(reqFile, argv) == -1)
	if (execve(reqFile, argv, environ) == -1)
	{
		printf("exec error: %s\n", strerror(errno));
	}

	return 0;
}

int start(void)
{
	char	ch;
	char	req[MAX_NUM] = "";
	char	reqFile[MAX_NUM] = "";
	int	httpVersion;
	FILE	*fin;
	int	size;
	struct stat bufStat;

	fgets(req, sizeof(req), stdin);
	sscanf(req, "GET /%s HTTP/1.%d", reqFile, &httpVersion);

	stat(reqFile, &bufStat);
	size = (int) bufStat.st_size;

	printf("HTTP/1.%d 200 OK\r\n", httpVersion);

	if (strcmp(reqFile + strlen(reqFile) - 5, ".html") == 0)
	{
		printf("Content-Type: text/html\r\n\r\n");
	}
	else if (strcmp(reqFile + strlen(reqFile) - 4, ".jpg") == 0)
	{
		printf("Content-Type: image/jpeg\r\n");
		printf("Content-Length: %d\r\n\r\n", size);
	}
	else if (strcmp(reqFile + strlen(reqFile) - 4, ".png") == 0)
	{
		printf("Content-Type: image/png\r\n");
		printf("Content-Length: %d\r\n\r\n", size);
	}
	else if (strcmp(reqFile + strlen(reqFile) - 4, ".gif") == 0)
	{
		printf("Content-Type: image/gif\r\n");
		printf("Content-Length: %d\r\n\r\n", size);
	}
	//else if (strcmp(reqFile + strlen(reqFile) - 4, ".cgi") == 0)
	else
	{
		cgi_handler(reqFile);
		return 0;
	}

	fin = fopen(reqFile, "rb");
	while (size--) {
		ch = getc(fin);
		putc(ch, stdout);
	}

	return 0;
}

int main(int argc, char **argv)
{
	int cpid;
	int serv_sock, cli_sock, cli_len;
	struct sockaddr_in cli_addr;

	serv_sock = passivesock("8080", "tcp", 30);

	while (1) {
		cli_len = sizeof(cli_addr);
		cli_sock = accept(serv_sock, (struct sockaddr *) &cli_addr, &cli_len);
		if (cli_sock < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}
			errexit("accept: %s\n", sys_errlist[errno]);
		}

		cpid = fork();
		if (cpid == -1)
		{
			exit(1);
		}
		else if (cpid == 0)	// child process
		{
			close(serv_sock);
			close(0);
			close(1);
			dup(cli_sock);
			dup(cli_sock);

			start();

			exit(0);
		}
		else			// parent process
		{
			printf("Connection from %s:%d.\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

			close(cli_sock);
		}
	}

	return 0;
}

