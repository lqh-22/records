/*
 * 头文件说明:
 *
 * #include <stdio.h>
 *   - 提供标准输入输出操作的功能，例如 printf 和 scanf。
 *
 * #include <stdlib.h>
 *   - 包含内存分配、进程控制、转换以及其他实用功能，例如 malloc、free 和 exit。
 *
 * #include <string.h>
 *   - 包含用于操作 C 字符串和内存块的函数，例如 strcpy、strlen 和 memset。
 *
 * #include <unistd.h>
 *   - 声明标准符号常量和类型，并提供对 POSIX 操作系统 API 的访问，例如 close、read 和 write。
 *
 * #include <arpa/inet.h>
 *   - 提供用于网络操作的定义，例如 IP 地址和端口号的转换函数，如 inet_pton 和 htons。
 *
 * #include <sys/socket.h>
 *   - 包含用于套接字编程的定义，包括套接字创建、绑定、监听和接受连接。
 *
 * #include <netinet/in.h>
 *   - 定义了互联网域地址所需的常量和结构，例如 sockaddr_in 和 INADDR_ANY。
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/select.h>

#define BACKLOG 10  // TCP最大连接数
#define max(a,b) ((a) > (b) ? (a) : (b))

char time_str[64];  // 存储目前时间
int sockClientFdList[1024], cntList; // 用于存储客户端套接字列表

char *getTime();  // 获得目前时间
/*
    服务器端代码
    1.创建TCP套接字
    2.绑定套接字到指定的IP地址和端口号
    3.监听连接请求
    4.接受连接请求
    5.接收数据
    6.处理数据
    7.发送数据
    8.关闭套接字
*/
int main(int argc, char const *argv[]) {
    // 格式判断
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }
    for (int i = 0; i < 1024; ++i) sockClientFdList[i] = -1;
    int socket_fd, client_fd;
    struct sockaddr_in serveraddr, clientaddr;
    char recv_buf[200] = "", send_buf[200] = "";
    socklen_t addrlen = sizeof(serveraddr);

    // 创建socket
    if ((socket_fd = socket(AF_INET,SOCK_STREAM,0)) == -1) {
        perror("fail to build socket");
        exit(1);
    }

    // 填充服务器网络数据结构
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
    serveraddr.sin_port = htons(atoi(argv[2])); // 使用 atoi 将字符串转换为整数
    bzero(&(serveraddr.sin_zero), 8);

    // 绑定socket
    if ((bind(socket_fd, (struct sockaddr *)&serveraddr, addrlen)) == -1) {
        perror("fail to bind");
        exit(1);
    }
    
    // 监听连接请求
    if ((listen(socket_fd, BACKLOG)) == -1) {
        perror("listen failed");
        exit(1);
    }

    /**
     * 使用select监听socket_fd、stdin、sockClientFdList
     *  socket_fd用于接受连接请求
     *  stdin用于发送消息
     *  sockClientFdList用于接受消息套接字
     */
    fd_set listenList, cpyList;
    FD_ZERO(&listenList);
    FD_SET(fileno(stdin), &listenList);
    FD_SET(socket_fd, &listenList);
    int maxFd = max(fileno(stdin), socket_fd) + 1;
    while (1) {
        cpyList = listenList;
        int res = select(maxFd, &cpyList, NULL, NULL, NULL);
        if (res < 1) {
            perror("select failed");
            break;
        }
        for (int i = 0; i < maxFd; ++i) {
            if (FD_ISSET(i, &cpyList)) {
                if (i == fileno(stdin) && cntList) {
                    // 处理标准输入
                    fgets(send_buf, sizeof(send_buf), stdin);
                    send_buf[strlen(send_buf) - 1] = '\0'; // 去掉换行符
                    int j = 0;
                    while (j < cntList) {
                        if (sockClientFdList[j] == -1) {
                            ++j;
                            continue;
                        }
                        if ((send(sockClientFdList[j], send_buf, sizeof(send_buf), 0)) == -1) {
                            perror("send failed");
                            exit(1);
                        }
                        ++j;
                    }
                } else if (i == socket_fd) {
                    // 接受连接请求
                    socklen_t clientlen = sizeof(clientaddr);
                    client_fd = accept(socket_fd, (struct sockaddr *)&clientaddr, &clientlen);
                    if (client_fd == -1){
                        perror("accept failed");
                        exit(1);
                    }
                    for (int i = 0; i < 1024; ++i) {
                        if (sockClientFdList[i] == -1) {
                            sockClientFdList[i] = client_fd;
                            ++cntList;
                            break;
                        }
                    }
                    FD_SET(client_fd, &listenList);
                    // 这个很重要，需要实时更新最大文件句柄
                    maxFd = max(maxFd, client_fd) + 1;
                    getTime();
                    fprintf(stdout, "\n[%s]:连接客户端成功\n  ip:%s  port:%d\n", 
                            time_str, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
                    // 发送连接成功信息
                    sprintf(send_buf, "[%s]:连接服务端成功\n  ip:%s  port:%d\n", time_str, argv[1], atoi(argv[2]));
                    if((send(client_fd, send_buf, sizeof(send_buf), 0)) == -1) {
                        perror("fail to send");
                        exit(1);
                    }
                } else {
                    int recvBytes = recv(i, recv_buf, sizeof(recv_buf), 0);
                    if (recvBytes == -1) {
                        perror("recv failed");
                        exit(1);
                    } else if (recvBytes == 0) {
                        // 客户端关闭连接
                        close(i);
                        FD_CLR(i, &listenList);
                        for (int j = 0; j < cntList; ++j) {
                            if (sockClientFdList[j] == i) {
                                sockClientFdList[j] = -1;
                                --cntList;
                                break;
                            }
                        }
                        fprintf(stdout, "[%s]:客户端已断开连接\n", getTime());
                    } else {
                        recv_buf[recvBytes] = '\0'; // 确保字符串以 '\0' 结尾
                        getTime();
                        fprintf(stdout, "\n[%s]:%s\n", time_str, recv_buf);
                    }
                }
            }
        }
    }
    close(client_fd);
    close(socket_fd);
    return 0;
}


char *getTime() {
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
    return time_str;
}