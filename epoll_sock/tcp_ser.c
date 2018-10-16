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
#include <sys/epoll.h>
#include <pthread.h>
#include <arpa/inet.h>

#define DEBUG 1 

#define PORT 8000
#define IP "127.0.0.1"
#define MAX_EVENT_NUMBER 1024
#define BUFFER_SIZE 10

//设置非阻塞
void setnoblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
}

/*
 * 将文件描述符fd上的EPOLLIN注册到epollfd指示的epoll内核事件表中
 * 参数enble_et表示是否对fd启用ET模式
 * LT模式，默认模式，事件发生，应用程序不立即处理事件，下次调用epoll_wait时，再次通知应用程序，直到处理
 * ET模式，当事件发生，epoll_wait检查到其上有事件发生并将事件通知应用程序后，应用程序就必须立即处理
 */
void addfd(int epollfd, int fd, int enable_et)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN;
	if (enable_et)
	{
		event.events |= EPOLLET;
	}
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	setnoblocking(fd);
}

//LT模式工作流程
void lt(struct epoll_event *events, int number, int epollfd, int listenfd)
{
	char buf[BUFFER_SIZE];

	for(int i=0; i<number; i++)
	{
		int sockfd = events[i].data.fd;

		//监听套接字描述符就绪,建立连接
		if (sockfd == listenfd)
		{

			//连接 
			struct sockaddr_in cliaddr;
			socklen_t clilen = sizeof(cliaddr);
			listenfd = accept(sockfd, (struct sockaddr *)&cliaddr, &clilen);
			if(-1 == listenfd)
			{
				printf("accept error:%s(error: %d)\n", strerror(errno), errno);
				exit(0);
			}
#ifdef DEBUG
			printf("[accept ok]\n");
#endif
			addfd(epollfd, listenfd, 0); //对listenfd禁用ET模式
		}

		//数据可读就绪,读缓冲区数据
		else if (events[i].events & EPOLLIN)
		{
			//只要socket读缓冲区还有未读数据,这段代码就被多次触发
			printf("enent trigger once\n");
			memset(buf, '\0', BUFFER_SIZE);
			int ret = recv(sockfd, buf, BUFFER_SIZE-1, 0);
			if (ret < 0)
			{
				close(sockfd);
				continue;
			}
			printf("get %d bytes of content: %s\n", ret, buf);
		}
		else
		{
			printf("something else happened\n");
		}
	}
}

//ET模式工作流程
void et(struct epoll_event *events, int number, int epollfd, int listenfd)
{
	char buf[BUFFER_SIZE];

	for(int i=0; i<number; i++)
	{
		int sockfd = events[i].data.fd;

		//监听套接字描述符就绪,建立连接
		if (sockfd == listenfd)
		{

			//连接 
			struct sockaddr_in cliaddr;
			socklen_t clilen = sizeof(cliaddr);
			listenfd = accept(sockfd, (struct sockaddr *)&cliaddr, &clilen);
			if(-1 == listenfd)
			{
				printf("accept error:%s(error: %d)\n", strerror(errno), errno);
				exit(0);
			}
#ifdef DEBUG
			printf("[accept ok]\n");
#endif
			addfd(epollfd, listenfd, 1); //对listenfd启用ET模式
		}

		//数据可读就绪,读缓冲区数据
		else if (events[i].events & EPOLLIN)
		{
			//这段代码不会重发触发，所有需要循环读取数据，确保所有数据读完
			printf("enent trigger once\n");
			while(1)
			{
				memset(buf, '\0', BUFFER_SIZE);
				int ret = recv(sockfd, buf, BUFFER_SIZE-1, 0);
				if (ret < 0)
				{
					/* 对于非阻塞IO,下面的条件成立表示数据已经完成全部数据读取
					 * 此后,epoll就能再次触发sockfd上的EPOLLIN事件,以驱动下一次读操作
					 */
					if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
					{
						printf(" read later\n");
						break;
					}
					close(sockfd);
					break;
				}
				else if (0 == ret)
				{
					close(sockfd);
				}
				else
				{
					printf("get %d bytes of content: %s\n", ret, buf);
				}
			}
		}
		else
		{
			printf("something else happened\n");
		}
	}
}

int main()
{
	int sockfd, ret;
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

	struct epoll_event events[MAX_EVENT_NUMBER];
	//创建内核事件表
	int epollfd = epoll_create(5); 
	addfd(epollfd, sockfd, 1);

	while(1)
	{
		ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
		if (-1 == ret)
		{
			printf("epoll_wati error(%d):%s\n", errno, strerror(errno));
			break;
		}

		//lt(events, ret, epollfd, sockfd); //使用LT模式
		et(events, ret, epollfd, sockfd); //使用ET模式
	}
	close(sockfd);

	return 0;
}

