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

int main()
{
    int sockfd, connect_fd, ret;
    struct sockaddr_in socketaddr;
    char buff[40];

	int filefd = open("hello", O_RDONLY);
	struct stat stat_buf;
	fstat(filefd, &stat_buf);

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
    memset(&socketaddr, 0, sizeof(socketaddr));
    socketaddr.sin_family = AF_INET;
    socketaddr.sin_port = htons(PORT);
    //inet_aton("127.0.0.1", &socketaddr.sin_addr.s_addr);
    socketaddr.sin_addr.s_addr = htonl(INADDR_ANY);
 
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

    //read or write
	sendfile(connect_fd, filefd, NULL, stat_buf.st_size);

    close(sockfd);
    close(connect_fd);

    return 0;
}

