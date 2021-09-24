#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 30

int main(int argc, char *argv[])
{
	fd_set reads, temps;// 声明read事件的文件描述符数组fd_set
	int result, str_len;
	char buf[BUF_SIZE];
	struct timeval timeout;

	/*1 设置文件描述符*/
	// 初始化
	FD_ZERO(&reads);
	// 将文件描述符0加入fd_set数组--监视标准输入的变化
	// 将fd为0的文件描述符对应的位置置为1
	FD_SET(0, &reads); // 0 is standard input(console)

	/*
	timeout.tv_sec=5;
	timeout.tv_usec=5000;
	*/

	while(1)
	{
        /*2 设置监视范围及超时*/
        // 将准备好的fd_set变量reads的内容复制到temps变量--调用select函数后，
        // 除发生变化的文件描述符对应位外，剩下的所有位将初始化为0
        // 为了记住初始值，使用该种复制的方式
		temps=reads;
		timeout.tv_sec=5;
		timeout.tv_usec=0;
		/*3 调用select函数并查看结果*/
		// 调用select函数
		result=select(1, &temps, 0, 0, &timeout);// select()返回值为发生相应事件  的socket数
		if(result==-1)
		{
			puts("select() error!");
			break;
		}
		else if(result==0)// 超时
		{
			puts("Time-out!");
		}
		else// 返回大于0的整数，说明相应数量的套接字/文件描述符发生了变化
		{
		    // 通过select的第二个参数传递的集合中存在需要读数据的描述符/套接字
			if(FD_ISSET(0, &temps)) // 值认为1的位置上的文件描述符发生了变化
			{
				str_len=read(0, buf, BUF_SIZE);
				buf[str_len]=0;
				printf("message from console: %s", buf);
			}
		}
	}
	return 0;
}