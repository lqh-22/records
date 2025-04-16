#include <stdio.h>      // 标准输入输出头文件，用于printf、scanf等函数
#include <stdlib.h>     // 标准库头文件，用于malloc、free、exit等函数
#include <string.h>     // 字符串操作头文件，用于strlen、strcpy等函数
#include <unistd.h>     // UNIX标准头文件，用于read、write、close等系统调用
#include <arpa/inet.h>  // 提供IP地址转换函数和网络编程相关定义
#include <sys/socket.h> // 套接字编程头文件，用于socket、bind、listen等函数
#include <time.h>       // 时间相关头文件，用于获取和操作时间
#include <pthread.h>   // 线程编程头文件，用于创建和管理线程
#include <netinet/in.h> // 提供互联网地址族的定义和结构体
#include <sys/select.h>  // 提供select函数的头文件

#define max(a,b) ((a) > (b) ? (a) : (b))

char time_str[64];  // 存储目前时间

char *getTime();

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
int main(int argc, char const *argv[]) {
    // 格式判断
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }
    int socked_fd;
    struct sockaddr_in serveraddr;
    char recv_buf[200] = "", send_buf[200] = "";

    // 创建套接字
    if((socked_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("fail to build socket");
        exit(1);
    }

    // 填充服务器网络数据结构
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
    serveraddr.sin_port = htons(atoi(argv[2]));
    bzero(&(serveraddr.sin_zero), 8);

    // 连接
    if (connect(socked_fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
        perror("connect failed");
        exit(1);
    }
    /**
     * 使用select将stdin和socked_fd加入监听，处理recv和send
     *  stdin用于发送消息
     *  socked_fd用于接受消息
     */

    fd_set listenList, cpyList;
    FD_ZERO(&listenList);
    FD_SET(fileno(stdin), &listenList);
    FD_SET(socked_fd, &listenList);
    int maxFd = max(fileno(stdin), socked_fd) + 1;
    while (1) {
        cpyList = listenList;
        int res = select(maxFd, &cpyList, NULL, NULL, NULL);
        if (res < 1) {
            perror("select failed");
            break;
        }
        for (int i = 0; i < maxFd; ++i) {
            if (FD_ISSET(i, &cpyList)) {
                if (i == fileno(stdin)) {
                    // stdin有数据
                    fgets(send_buf, sizeof(send_buf), stdin);
                    // fgets会将换行符读入，将最后的换行符替代为\0
                    send_buf[strlen(send_buf) - 1] = '\0';
                    if ((send(socked_fd, send_buf, sizeof(send_buf), 0) == -1)) {
                        perror("send failed");
                        break;
                    }
                    if (strcmp(send_buf, "exit") == 0) {
                        fprintf(stdout, "exit\n");
                        close(socked_fd);
                        exit(0);
                    }
                } else if (i == socked_fd) {
                    // 服务器端有数据
                    int recvBytes = recv(socked_fd, recv_buf, sizeof(recv_buf), 0);
                    if (recvBytes == -1) {
                        perror("recv failed");
                        break;
                    } else if (recvBytes == 0) {
                        fprintf(stdout, "server closed\n");
                        close(socked_fd);
                        exit(0);
                    } else {
                        recv_buf[recvBytes] = '\0';
                        getTime();
                        fprintf(stdout, "\n[%s]:%s\n", time_str, recv_buf);
                    }
                }
            }
        }
    }
    close(socked_fd);
    
    return 0;
}

char *getTime() {
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
    return time_str;
}