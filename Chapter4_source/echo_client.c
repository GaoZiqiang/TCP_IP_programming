#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

// 自定义数据包结构
struct Packet
{
    unsigned int      msgLen;     //数据部分的长度(注：这是网络字节序)
    char            text[1024]; //报文的数据部分
};

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int sock;
	char message[BUF_SIZE];
	int str_len;
	struct sockaddr_in serv_adr;

	if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_STREAM, 0);   
	if(sock==-1)
		error_handling("socket() error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));
	
	if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("connect() error!");
	else
		puts("Connected...........");
	
	while(1) 
	{
        struct Packet buf;
        memset(&buf, 0, sizeof(buf));
		fputs("Input message(Q to quit): ", stdout);
//		fgets(message, BUF_SIZE, stdin);

		if (!fgets(buf.text, sizeof(buf.text), stdin))
		    return -1;
        // 输入'Q' or 'q' 退出循环
		if(!strcmp(buf.text,"q\n") || !strcmp(buf.text,"Q\n"))
			break;

		unsigned int lenHost = strlen(buf.text);
		buf.msgLen = htonl(lenHost);

		write(sock, &buf, sizeof(buf.msgLen) + lenHost);// msgLen的长度+text的长度
		printf("发消息发送完毕!\n");

//		sleep(5);
		// 接收来自server端的数据
		memset(&buf, 0, sizeof(buf));
		// 先读取buf.msgLen字段
        read(sock, &buf.msgLen, sizeof(buf.msgLen));
        lenHost = ntohl(buf.msgLen);
        printf("lenHost from server: %d\n", lenHost);
        // 再根据msgLen读取buf.text字段
        int read_bytes = read(sock, buf.text, lenHost);
        if (read_bytes == -1) {
            error_handling("read error\n");
        } else if (read_bytes != lenHost) {
            error_handling("client connect closed...\n");
        }
        printf("receive from server: %s\n", buf.text);

//        int ret = 0;
//        int i = 0;
//        while((ret = read(sock, &buf.msgLen, sizeof(buf.msgLen))) > 0) {
//            printf("第 %d 轮\n", ++i);
//            // 再根据msgLen读取buf.text字段
//            int lenHost = ntohl(buf.msgLen);
//            printf("lenHost from server: %d\n", lenHost);
//            int read_bytes = read(sock, buf.text, lenHost);
//            if (read_bytes == -1) {
//                error_handling("read error\n");
//            } else if (read_bytes != lenHost) {
//                error_handling("client connect closed...\n");
//            }
//            printf("receive from server: %s\n", buf.text);
//        }

//        fputs("Input message(Q to quit): ", stdout);
//        fgets(message, BUF_SIZE, stdin);
//
//		// 输入'Q' or 'q' 退出循环
//		if(!strcmp(message,"q\n") || !strcmp(message,"Q\n"))
//			break;
//
//		// 问题：两个message变量是一样的？如何区分来自server端还是自己发送的？--这个问题貌似很外行
//		write(sock, message, strlen(message));
//		// 这种处理方式存在问题：一条消息太长，服务器端可能分多次发送，因此客户端需要分多次接受才能接收到完整的一条消息
//		str_len=read(sock, message, BUF_SIZE-1);
//		message[str_len]=0;// 为什么？--添加数组结束符？为什么要单独加一个这个呢？
//		printf("Message from server: %s", message);
	}
	
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}