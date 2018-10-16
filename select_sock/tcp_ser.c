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
#include <sys/time.h>
#include <sys/select.h>

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

	fd_set read_fds;
	fd_set exception_fds;
	FD_ZERO(&read_fds);
	FD_ZERO(&exception_fds);
	struct timeval tm;

	while(1)
	{
		memset(buff, '\0', sizeof(buff));
		FD_SET(connect_fd, &read_fds);
		FD_SET(connect_fd, &exception_fds);
		tm.tv_sec = 5;
		tm.tv_usec = 0;

		ret = select(connect_fd+1, &read_fds, NULL, &exception_fds, &tm);
		if(ret < 0)
		{
			printf("select error:%s\n", strerror(errno));
			break;
		}
		if(0 == ret)
		{
			printf("timeout\n");	
			exit(0);
		}
		if(FD_ISSET(connect_fd, &read_fds))
		{
			ret = recv(connect_fd, buff, sizeof(buff), 0);
			if( ret <= 0)
				break;
			printf("buff is %d %s\n", ret, buff);
		}
		if(FD_ISSET(connect_fd, &exception_fds))
		{
			ret = recv(connect_fd, buff, sizeof(buff), MSG_OOB);
		}
	}

	close(sockfd);
	close(connect_fd);

	return 0;
}

