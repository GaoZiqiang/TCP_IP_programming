#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define BUF_SIZE 100
#define EPOLL_SIZE 50
void error_handling(char *buf);

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	socklen_t adr_sz;
	int str_len, i;
	char buf[BUF_SIZE];

	struct epoll_event *ep_events;// ep_events:活跃套接字集
	struct epoll_event event;
	int epfd, event_cnt;

	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	serv_sock=socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");

	// 创建epoll例程--只有一个吗？--只需创建一个吗？
	epfd=epoll_create(EPOLL_SIZE);
	// 为ep_events数组申请内存空间
	ep_events=malloc(sizeof(struct epoll_event)*EPOLL_SIZE);

	event.events=EPOLLIN;// 注册事件
	event.data.fd=serv_sock;// 注册sock fd
    // 注册serv_sock到epfd
	epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);

	while(1)
	{
	    // 监视事件的发生
	    // 返回值event_cnt为发生事件的fd个数
	    // 第二个参数ep_events保存发生事件的fd的集合/缓冲区--ep_events为该集合/缓冲区的地址--将发生的事件放到ep_events
		event_cnt=epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
		if(event_cnt==-1)
		{
			puts("epoll_wait() error");
			break;
		}

		for(i=0; i<event_cnt; i++)
		{
		    // sockfd
			if(ep_events[i].data.fd==serv_sock)
			{
				adr_sz=sizeof(clnt_adr);
				clnt_sock=
					accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
				// 将clnt_sock注册到epoll
				event.events=EPOLLIN;
				event.data.fd=clnt_sock;
				epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);// 事件类型跟sockfd的为同一个--event
				printf("connected client: %d \n", clnt_sock);
			}
			// client fd--处理活跃套接字集合
			else
			{
			        // client_fd = ep_events[i].data.fd
					str_len=read(ep_events[i].data.fd, buf, BUF_SIZE);
					if(str_len==0)    // close request!
					{
					    // EPOLL_CTL_DEL:将关闭的clnt_sock清除出epoll
						epoll_ctl(
							epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
						close(ep_events[i].data.fd);
						printf("closed client: %d \n", ep_events[i].data.fd);
					}
					else
					{
					    buf[str_len] = 0;
					    printf("From client %d:%s", ep_events[i].data.fd,buf);
						write(ep_events[i].data.fd, buf, str_len);    // echo!
					}
	
			}
		}
	}
	close(serv_sock);
	close(epfd);
	return 0;
}

void error_handling(char *buf)
{
	fputs(buf, stderr);
	fputc('\n', stderr);
	exit(1);
}
