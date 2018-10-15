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
#include <assert.h>

#define DEBUG 1 

#define PORT 8000
#define IP "127.0.0.1"

int main()
{
	int sockfd, connect_fd, ret;
	struct sockaddr_in socketaddr;
	char buff[40];

	int filefd = open("hello", O_RDWR);

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

	//tee在两个管道描述符之间复制数据
	int pipefd[2];
	ret = pipe(pipefd);
	int pipefd_file[2];
	ret = pipe(pipefd_file);
	assert(ret != -1);
	//将从connect_fd上流入的客户端数据定向到管道中
	ret = splice(connect_fd, NULL, pipefd[1], NULL, 32768, 0);
	//将pipefd的输出复制到管道pipefd的输入端
	ret = tee(pipefd[0], pipefd_file[1], 32768, 0);
	//将从管道中的输出数据定向到connect_fd客户端连接文件描述符中
	ret = splice(pipefd[0], NULL, filefd, NULL, 32768, 0);
	ret = splice(pipefd_file[0], NULL, STDOUT_FILENO, NULL, 32768, 0);

	close(filefd);
	close(pipefd[0]);
	close(pipefd[1]);
	close(pipefd_file[0]);
	close(pipefd_file[1]);
	close(sockfd);
	close(connect_fd);

	return 0;
}

