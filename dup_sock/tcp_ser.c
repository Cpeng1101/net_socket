#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/sendfile.h>
#include <fcntl.h>

#define DEBUG 1 

#define PORT 8000
#define IP "127.0.0.1"

int main()
{
	int sockfd, connect_fd, ret;
	struct sockaddr_in socketaddr;
	char buff[40];

	//初始化socket 
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockfd)
	{
		printf("create socket error:%s(error: %d)\n", strerror(errno), errno);
		exit(0);
	}
#ifdef DEBUG
	printf("[socket ok]\n");
#endif

	//初始化
	memset(&socketaddr, '\0', sizeof(socketaddr));
	socketaddr.sin_family = AF_INET;
	socketaddr.sin_port = htons(PORT);
	//inet_aton("127.0.0.1", &socketaddr.sin_addr);
	//socketaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	inet_pton(AF_INET, IP, &socketaddr.sin_addr);

	//绑定bind 
	ret = bind(sockfd, (struct sockaddr *)&socketaddr, sizeof(socketaddr));
	if (-1 == ret)
	{
		printf("bind error:%s(error: %d)\n", strerror(errno), errno);
		exit(0);
	}
#ifdef DEBUG
	printf("[bind ok]\n");
#endif

	//监听
	listen(sockfd, 10);

	//连接 
	connect_fd = accept(sockfd, NULL, NULL);
	if(-1 == connect_fd)
	{
		printf("accept error:%s(error: %d)\n", strerror(errno), errno);
		exit(0);
	}
#ifdef DEBUG
	printf("[accept ok]\n");
#endif

	//CGI编程基本原理
	close(STDOUT_FILENO);  //关闭标准输出
	dup(connect_fd);	   //复制描述符为1
	printf("hello word\n");//在客户端显示
	close(connect_fd);

	close(sockfd);

	return 0;
}

