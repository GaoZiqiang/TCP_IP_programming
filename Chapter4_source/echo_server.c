#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>          /* See NOTES */
//#include <sys/socket.h>

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
	int serv_sock, clnt_sock;
	char message[BUF_SIZE];
	int str_len, i;
	
	struct sockaddr_in serv_adr;
	struct sockaddr_in clnt_adr;
	socklen_t clnt_adr_sz;
	
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	serv_sock=socket(PF_INET, SOCK_STREAM, 0);
//	int on = 1;
//    setsockopt (serv_sock, SOL_SOCKET,SO_REUSEADDR,TCP_NODELAY, (void *) &on, sizeof (on));
	if(serv_sock==-1)
		error_handling("socket() error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");

	// open a new socket for the coming connection,and return new socket's descriptor, or -1 for errors
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	else
	    printf("listened client nums: %d\n",listen(serv_sock, 5));
	
	clnt_adr_sz=sizeof(clnt_adr);

	/*最多处理五个client端请求*/
	// 有点鸡肋
	for(i=0; i<5; i++)
	{
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
		if(clnt_sock==-1)
			error_handling("accept() error");
		else
			printf("Connected client %d \n", i+1);

        struct Packet buf;
        memset(&buf, 0, sizeof(buf));
        // 先读msgLen这个字段
        int ret = 0;
        while((ret = read(clnt_sock, &buf.msgLen, sizeof(buf.msgLen))) > 0) {
            // 再根据msgLen读取buf.text字段
            int lenHost = ntohl(buf.msgLen);
            printf("lenHost from client %d: %d\n", clnt_sock, lenHost);
            int read_bytes = read(clnt_sock, buf.text, lenHost);
            if (read_bytes == -1) {
                error_handling("read error\n");
            } else if (read_bytes != lenHost) {
                error_handling("client connect closed...\n");
            }
            printf("receive from client %d: %s", clnt_sock, buf.text);

            // 写回client
            if (write(clnt_sock, &buf, sizeof(buf.msgLen) + lenHost) == -1) {
                error_handling("write socket error\n");
            }
        }





//		while((str_len=read(clnt_sock, message, BUF_SIZE))!=0) {
//            printf("Message from client %d :%s\n",i+1,message);
//		    write(clnt_sock, message, str_len);
////            message[str_len]=0;
//		}

		close(clnt_sock);
	}

	close(serv_sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}