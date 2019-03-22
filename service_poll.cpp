#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <malloc.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <fcntl.h>
#include <fcntl.h> 
#include <sys/poll.h>  
#define BACKLOG 100  
 
#if 0
struct pollfd
{
　　	int fd;          //文件描述符
　　	short events;    //请求的事件
　　	short revents;   //返回的事件
　　};
#endif
 
 
int main()  
{
    int iListenSock = socket(AF_INET, SOCK_STREAM, 0);
    int arrayConnSock[BACKLOG] = {0}; // 这个初始化并不好，但其实无大碍
    sockaddr_in addr;  
    memset(&addr, 0, sizeof(addr));  
    inet_aton("0.0.0.0", &addr.sin_addr);  
    addr.sin_family = AF_INET;  
    addr.sin_port = htons(8888);  
 
	int iOpt = 1;
    setsockopt(iListenSock, SOL_SOCKET, SO_REUSEADDR, &iOpt, sizeof(iOpt));  // 标配
    bind(iListenSock, (sockaddr*)&addr, sizeof(addr));  
    listen(iListenSock, 100);  
  
	int flag = -1;
	pollfd fds[BACKLOG + 1];        // BACKLOG个通信socket和1个监听socket
	memset(fds, flag, sizeof(fds)); // 设置一个标志
	fds[0].fd = iListenSock;        // 监听socket
	fds[0].events = POLLIN;
 
	int ndfs = 1;
	int timeoutMS = -1; // 永不超时
	int i = 0;
	for(int i = 1; i <= BACKLOG; i++)
	{  
	    int iRet = poll(fds, ndfs, timeoutMS);  
	    if (iRet < 0)
		{
			printf("poll error, iRet %d\n", iRet);
	        continue;  
	    }
 
		if (iRet < 0)
		{
			printf("poll error, iRet %d\n", iRet);
	        continue;  
	    }
 
	    if (fds[0].revents & POLLIN) // fds[0]对应iListenSock， 此处表明检测到有新客户端连接过来
		{
            arrayConnSock[i - 1] = accept(iListenSock, NULL, NULL); // arrayConnSock是通信socket
            fds[i].fd = arrayConnSock[i - 1];  
            fds[i].events = POLLIN;
			ndfs++;  
            printf("new client came, local fd is [%d]\n", arrayConnSock[i - 1]);  
	    }  
 
	    for (int j = 0; j < BACKLOG; j++)
		{
			char szBuf[1024] = {0}; 
	        if (arrayConnSock[j] > 0 && (fds[j + 1].revents & POLLIN)) // 此处检测的是已经链接的socket的读事件
			{  
	            int recvLen = recv(arrayConnSock[j], szBuf, sizeof(szBuf) - 1, 0);  
	            if (recvLen > 0)
				{  
	                printf("recv data [%s] from local fd [%d] \n", szBuf, arrayConnSock[j]);  
	            }
				else if(0 == recvLen)
	            {
	                close(arrayConnSock[j]);  
	                memset(&fds[j + 1], flag, sizeof(pollfd));  
	                printf("connection closed fd [%d]\n", arrayConnSock[j]);  
	                arrayConnSock[j] = 0; 
	            }
				else
				{
					close(arrayConnSock[j]);  
	                memset(&fds[j + 1], flag, sizeof(pollfd));  
					printf("recv error, recvLen is %d", recvLen);
					arrayConnSock[j] = 0;
				}
	        }  
	    }  
	
	}
  
    close(iListenSock);  
    return 0;  
}
