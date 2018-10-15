#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>
#include <assert.h>
#include <netdb.h>

int main(int argc, char** argv)
{
	int sockfd, ret;
	struct sockaddr_in serv_addr;
	char buff[40];

	assert(2 == argc);
	char *host = argv[1];
	//获取主机地址信息
	struct hostent *hostinfo = gethostbyname(host);
	assert(hostinfo);
	
	//获取daytime服务信息
	struct servent *servinfo = getservbyname("daytime", "tcp");
	assert(servinfo);
	printf("daytime port is %d\n", ntohs(servinfo->s_port));

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
	serv_addr.sin_port = servinfo->s_port;
	serv_addr.sin_addr = *(struct in_addr *)*hostinfo->h_addr_list;

	//连接 
	ret = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if(-1 == ret)
	{
		printf("accept error:%s(error: %d)\n", strerror(errno), errno);
		exit(0);
	}

	//read or write
	char buffer[128];
	ret = read(sockfd, buffer, sizeof(buffer));
	assert(ret > 0);
	buffer[ret] = '\0';
	printf("the day time is: %s", buffer);

	close(sockfd);
	return 0;
}

