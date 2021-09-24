#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <iostream>

#define BUF_SIZE 100
void error_handling(char *buf);

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	struct timeval timeout;
	fd_set reads, cpy_reads;// 声明监测read事件的fd_set

	socklen_t adr_sz;
	int fd_max, str_len, fd_num, i;
	char buf[BUF_SIZE];
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	serv_sock=socket(PF_INET, SOCK_STREAM, 0);
	printf("serv_socket: %d\n",serv_sock);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	// 这个listen的作用到底是啥？
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");

	// 初始化fd_read
	FD_ZERO(&reads);
    // 将服务器端socket fd添加到被监控的fd_read数组
	FD_SET(serv_sock, &reads);
	// 这行又该怎么理解？
	fd_max=serv_sock;// fd_max:所监视的socket的fd的个数

	while(1)
	{
		cpy_reads=reads;
		timeout.tv_sec=5;
		timeout.tv_usec=5000;

		// 需要每次都传递监视对象信息--向操作系统传递--改进：只传递一次
		// 使用select()监视cpy_reads中各个socket fd的变化情况
		if((fd_num=select(fd_max+1, &cpy_reads, 0, 0, &timeout))==-1)// 后续判断为什么不适用fd_num呢？
			break;
		
		if(fd_num==0)
			continue;

		// 针对所有文件描述符的循环--处理？--改进：只通知/处理发生变化的事项
		// 这里的判断为什么不使用fd_num呢？--发生事件的fd
		for(i=0; i<fd_max+1; i++)
		{
		    // 发生变化的socket fd为原来fd_set数组中的socket
			if(FD_ISSET(i, &cpy_reads))
			{
			    printf("发生变化的fd: %d\n",i);
                // 如果监测到serv_sock的fd发生了变化，则表示服务器端接收到了客户端发来的连接请求--因此需要进行accept-connect
			    // 监测到serv_sock的fd发生变化

			    // server端接收到来自客户端的连接申请
				if(i==serv_sock)     // connection request!
				{
				    printf("serv_sock %d 发生了事件\n",i);
					adr_sz=sizeof(clnt_adr);
					// accept
					clnt_sock=
						accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
                    std::cout<<"ip="<<inet_ntoa(clnt_adr.sin_addr)<<
                             " port="<<ntohs(clnt_adr.sin_port)<<std::endl;
					// 将clnt_sock添加到fd_set数组中
					FD_SET(clnt_sock, &reads);
					if(fd_max<clnt_sock)
						fd_max=clnt_sock;
					printf("新连接的client: %d \n", clnt_sock);
				}
				// 如果select监测到发生变化的fd不是服务器端的fd serv_sock发生变化，通过排除法可以判定是客户端发送来了数据
				// 监测到客户端的fd有变化--客户端的fd通过FD_SET(clnt_sock, &reads);添加到fd_set了
				// 接收客户端的数据--服务器端来接收
				else    // read message!else if (i == clnt_sock)?
				{
                    printf("clnt_sock %d 发生了事件\n",i);
					str_len=read(i, buf, BUF_SIZE);

					// read到的数据str_len为0--表明客户端不再发送数据-->关闭client socket
					if(str_len==0)    // close request!
					{
					    // 将结束传输的客户端clnt_sock移除fd_set
						FD_CLR(i, &reads);
						close(i);
						printf("closed client: %d \n", i);
					}
					else
					{
					    // 输出到console
                        buf[str_len]=0;
                        printf("message from client %d: %s", i,buf);
                        // 发送回/写回client端
						write(i, buf, str_len);    // echo!
					}
				}
			}
		}
	}
	close(serv_sock);
	return 0;
}

void error_handling(char *buf)
{
	fputs(buf, stderr);
	fputc('\n', stderr);
	exit(1);
}