#include <stdio.h>      // 标准输入输出头文件，用于printf、scanf等函数
#include <stdlib.h>     // 标准库头文件，用于malloc、free、exit等函数
#include <string.h>     // 字符串操作头文件，用于strlen、strcpy等函数
#include <unistd.h>     // UNIX标准头文件，用于read、write、close等系统调用
#include <arpa/inet.h>  // 提供IP地址转换函数和网络编程相关定义
#include <sys/socket.h> // 套接字编程头文件，用于socket、bind、listen等函数
#include <time.h>       // 时间相关头文件，用于获取和操作时间
#include <pthread.h>   // 线程编程头文件，用于创建和管理线程
#include <netinet/in.h> // 提供互联网地址族的定义和结构体

#define BACKLOG 10  // TCP最大连接数
#define SERVER_PORT 9999  // 默认端口号
#define EXIT "exit"  // 退出命令

typedef struct {
    int sockFd; // 套接字
    char type[10];  // 标识客户端/服务器
    char *ip; // IP地址
} threadArgs;  // 用于传递多线程参数

typedef struct {
    int sockFd; // 套接字
    struct sockaddr_in clientAddr; // 客户端地址
} threadArgsServer;  // 主线程开辟多个子线程处理TCP连接

char sendBuf[200] = "", recvBuf[200] = "";
char time_str[64];  // 存储目前时间
FILE *fp; // 文件指针，用于记录聊天信息

void *PthreadRecv(void *arg);  // 新建一个子线程处理recv
char *getTime();  // 获得目前时间
void *handleClient(void *args);  // 新建一个子线程处理TCP连接


/*
- 这个程序既集成了客户端，也集成了服务器端：当没有参数时，程序是服务器端；当有作为IP地址和端口的参数时，程序是客户端。
- 程序的服务端运行一开始就会有提示，显示自己服务器端主机的IP地址，以方便客户端连接。
- 程序每次聊天信息的发出都会附加上时间，并且退出后会有聊天的记录和退出的记录。
- 程序中使用了多线程的方法，解决了程序阻塞的问题，使得聊天程序不用等待（回答和响应可同时运行）。
*/
int main(int argc, char const *argv[]) {
    if (argc != 1 && argc != 3) {
        fprintf(stdout, "Usage: %s or %s <ip> <port>\n", argv[0], argv[0]);
        exit(1);
    }
    /**
     * 服务端代码
     * TODO：服务端添加文件操作
     * 1.sockServer, sockClient
     * 2.bind
     * 3.listen
     * 4.accept
     * 5.send/recv
     * 注意：
     *  1.recv与send需要两个线程控制
     *  2.文件要记录
     *      [时间] 聊天者(服务端/客户端)：内容
     *      ----聊天者，离开了---
     */
    if (argc == 1) {
        struct sockaddr_in sockServer, sockClient;
        int sockServerFd, sockClientFd;
        socklen_t sockClientLen = sizeof(sockClient);
        sockServerFd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockServerFd == -1) {
            fprintf(stderr, "[%s] fail to create socket\n", getTime());
            exit(1);
        }
        // 需要将输入保存到日志中
        if ((fp = fopen("log_server.txt", "a+")) == NULL) {
            fprintf(stderr, "[%s] fail to open file\n", getTime());
            exit(1);
        }
        sockServer.sin_family = AF_INET;
        sockServer.sin_addr.s_addr = INADDR_ANY;  // 绑定到所有可用的IP地址
        sockServer.sin_port = htons(SERVER_PORT);
        bzero(&(sockServer.sin_zero), 8);
        if (bind(sockServerFd, (struct sockaddr *)&sockServer, sizeof(sockServer)) == -1) {
            fprintf(stderr, "[%s] fail to bind\n", getTime());
            exit(1);
        }
        //输出服务器的IP地址，端口号
        fprintf(stdout, "Server IP:");
        fflush(stdout);
        system("ifconfig | grep inet | head -n 1 | awk '{print $2}'");
        fprintf(stdout, "Server Port:%d\n", SERVER_PORT);
        if (listen(sockServerFd, BACKLOG) == -1) {
            fprintf(stderr, "[%s] fail to listen\n", getTime());
            exit(1);
        }
        // 循环accept，确保客户端能连接
        while (1) {
            if ((sockClientFd = accept(sockServerFd, (struct sockaddr *)&sockClient, &sockClientLen)) == -1) {
                fprintf(stderr, "[%s] fail to accept\n", getTime());
                continue;
            }
            //向客户端发送连接成功的消息，本地打印远程连接客户端的IP地址
            fprintf(stdout, "[%s] 成功连接到客户端：%s:%d\n", getTime(), inet_ntoa(sockClient.sin_addr), ntohs(sockClient.sin_port));
            strcpy(sendBuf, "成功连接到服务器");
            if (send(sockClientFd, sendBuf, sizeof(sendBuf), 0) == -1) {
                fprintf(stderr, "[%s] fail to send\n", getTime());
                exit(1);
            }
            /**
             * 需要开一个线程处理这个TCP连接的通信，主进程只需要连接TCP即可
             */
            // 为每个客户端创建线程
            pthread_t clientThread;
            // 将子线程需要的通信套接字、客户端地址传入
            // 我们需要Malloc一个结构体来传递参数,不可以直接传递局部变量，因为下一个线程会覆盖掉
            threadArgsServer *args = (threadArgsServer *)malloc(sizeof(threadArgsServer));
            if (args == NULL) {
                fprintf(stderr, "[%s] fail to allocate memory\n", getTime());
                close(sockClientFd);
                continue;
            }
            args->sockFd = sockClientFd;
            args->clientAddr = sockClient;
            if (pthread_create(&clientThread, NULL, handleClient, args) != 0) {
                fprintf(stderr, "[%s] fail to create thread\n", getTime());
                close(sockClientFd);
            } else {
                // 分离线程，避免资源泄漏
                pthread_detach(clientThread);
            }
        }
        close(sockServerFd);
        fclose(fp);
    } else {
        /**
         * 客户端代码
         * 1.sockServer
         * 2.connect
         * 3.send/recv
         * 注意：
         *  1.recv与send需要两个线程控制
         *  2.文件要记录
         *      [时间] 聊天者(服务端/客户端)：内容
         *      ----聊天者，离开了---
         */
        if ((fp = fopen("log_client.txt", "a+")) == NULL) {
            fprintf(stderr, "[%s] fail to open file\n", getTime());
            exit(1);
        }
        struct sockaddr_in sockServer;
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            fprintf(stderr, "[%s] fail to create socket\n", getTime());
            exit(1);
        }
        sockServer.sin_family = AF_INET;
        sockServer.sin_addr.s_addr = inet_addr(argv[1]);   
        sockServer.sin_port = htons(atoi(argv[2]));  // 使用 atoi 将字符串转换为整数
        bzero(&(sockServer.sin_zero), 8);
        if (connect(sockfd, (struct sockaddr *)&sockServer, sizeof(sockServer)) == -1) {
            fprintf(stderr, "[%s] fail to connect\n", getTime());
            exit(1);
        }
        // 在和服务器端建立连接后，需要获取本地IP
        struct sockaddr_in localAddr;
        socklen_t addrLen = sizeof(localAddr);
        if (getsockname(sockfd, (struct sockaddr *)&localAddr, &addrLen) == -1) {
            fprintf(stderr, "[%s] fail to get local IP\n", getTime());
            exit(1);
        }
        fprintf(stdout, "[%s] Client IP: %s, Client Port: %d\n", 
                getTime(), inet_ntoa(localAddr.sin_addr), ntohs(localAddr.sin_port));


        //接收连接成功的消息
        int recvBytes = recv(sockfd, recvBuf, sizeof(recvBuf), 0);
        if (recvBytes == -1) {
            fprintf(stderr, "[%s] fail to recv\n", getTime());
            exit(1);
        }
        recvBuf[recvBytes] = '\0';
        fprintf(stdout, "[%s] %s ServerIP: %s ServerPort: %d\n", getTime(), recvBuf, inet_ntoa(sockServer.sin_addr), ntohs(sockServer.sin_port));
        //多线程处理recv
        pthread_t pthreadId;
        threadArgs args = {sockfd, "server", inet_ntoa(sockServer.sin_addr)};
        if ((pthread_create(&pthreadId, NULL, PthreadRecv, &args)) != 0) {
            fprintf(stderr, "[%s] fail to create thread\n", getTime());
            exit(1);
        }  
        //当前进程处理send
        while (1) {
            fgets(sendBuf, sizeof(sendBuf), stdin);
            // fgets会将\n读入，需要替换为\0
            sendBuf[strlen(sendBuf) - 1] = '\0';
            // 记录聊天信息到文件
            fprintf(fp, "[%s] %s(%s): %s\n", getTime(), inet_ntoa(sockServer.sin_addr), args.type, sendBuf);
            fflush(fp);  // 实时刷新文件
            if ((send(sockfd, sendBuf, sizeof(sendBuf), 0)) == -1) {
                fprintf(stderr, "[%s] faile to send\n", getTime());
                exit(1);
            }
            if (strcmp(sendBuf, EXIT) == 0) {
                fprintf(stdout, "[%s] 客户端已退出\n", getTime());
                fprintf(fp, "[%s] 客户端已退出\n", getTime());
                fflush(fp);  // 实时刷新文件
                break;
            }
        }
        close(sockfd);
        fclose(fp);
    }
    return 0;
}

void *PthreadRecv(void *arg) {
    char str[200] = "";
    threadArgs args = *(threadArgs *)arg;
    int recvBytes = 0;
    while (1) {
        recvBytes = recv(args.sockFd, str, sizeof(str), 0);
        if (recvBytes == -1) {
            fprintf(stderr, "[%s] fail to recv\n", getTime());
            exit(1);
        } else if (strcmp(str, EXIT) == 0) {
            fprintf(stdout, "[%s] 对方(%s)已退出\n", getTime(), args.ip);
            fprintf(fp, "[%s] 对方(%s)已退出\n", getTime(), args.ip);
            fflush(fp);  // 实时刷新文件
            exit(0);  // 子线程不需要关闭文件，因为主线程要用
        } else {
            str[recvBytes] = '\0';
            fprintf(stdout, "[%s] %s(%s): %s\n", getTime(), args.ip, args.type, str);
            // 记录聊天信息到文件
            fprintf(fp, "[%s] %s(%s): %s\n", getTime(), args.ip, args.type, str);
            fflush(fp);  // 实时刷新文件
        }
    }
    pthread_exit((void *)1);
}

char *getTime() {
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
    return time_str;
}

void *handleClient(void *arg) {
    threadArgsServer argsServer = *(threadArgsServer *)arg;
    int sockClientFd = argsServer.sockFd;
    struct sockaddr_in sockClient = argsServer.clientAddr;
    //多线程处理recv
    pthread_t pthreadId;
    threadArgs args = {sockClientFd, "client", inet_ntoa(sockClient.sin_addr)};
    if ((pthread_create(&pthreadId, NULL, PthreadRecv, &args)) != 0) {
        fprintf(stderr, "[%s] fail to create thread\n", getTime());
        exit(1);
    }
    //当前进程处理send
    while (1) {
        fgets(sendBuf, sizeof(sendBuf), stdin);
        // fgets会将\n读入，需要替换为\0
        sendBuf[strlen(sendBuf) - 1] = '\0';
        // 记录聊天信息到文件
        fprintf(fp, "[%s] %s(%s): %s\n", getTime(), inet_ntoa(sockClient.sin_addr), args.type, sendBuf);
        fflush(fp);  // 实时刷新文件
        if ((send(sockClientFd, sendBuf, sizeof(sendBuf), 0)) == -1) {
            fprintf(stderr, "[%s] faile to send\n", getTime());
            exit(1);
        }
        if (strcmp(sendBuf, EXIT) == 0) {
            fprintf(stdout, "[%s] 服务端已退出\n", getTime());
            fprintf(fp, "[%s] 服务端已退出\n", getTime());
            fflush(fp);  // 实时刷新文件
            break;
        }
    }
    close(sockClientFd);
    pthread_exit((void *)1);
}