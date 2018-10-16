#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>

int main(int argc, char** argv)
{
	int sockfd, ret;
	struct sockaddr_in serv_addr;
	char buff[40];

	if(argc != 3)
	{
		printf("usage: %s <ipaddr+port>\n", argv[0]);
		exit(0);
	}

	//初始化socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockfd)
	{
		printf("create socket error:%s(error: %d)\n", strerror(errno), errno);
		exit(0);
	}

	//初始化
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	inet_aton(&argv[1], &serv_addr.sin_addr);
	serv_addr.sin_port = htons(atoi(argv[2]));

	//连接 
	ret = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if(-1 == ret)
	{
		printf("accept error:%s(error: %d)\n", strerror(errno), errno);
		exit(0);
	}

	//read or write
	//send(sockfd, "hello word", sizeof("hello word"), 0);
	while(1);

	close(sockfd);

	return 0;
}

