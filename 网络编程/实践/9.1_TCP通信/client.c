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

char time_str[64];  // 存储目前时间

char *getTime();  // 获得目前时间

/**
 * 客户端程序
 * 1.创建套接字
 * 2.连接服务器
 * 3.发送数据
 * 4.关闭套接字
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

    // 接受连接成功信息
    if (!fork()) {
        // 子进程代码
        int recv_len = 0;
        recv_len = recv(socked_fd, recv_buf, sizeof(recv_buf), 0);
        if (recv_len == -1) {
            perror("faile to recv");
            exit(1);
        }
        getTime();
        fprintf(stdout, "\n%s\n", recv_buf);
        exit(0);
    }

    // 通信
    int recv_len = 0;
    while (1) {
        fgets(send_buf, sizeof(send_buf), stdin);
        // fgets会将换行符读入，将最后的换行符替代为\0
        send_buf[strlen(send_buf) - 1] = '\0';
        if ((send(socked_fd, send_buf, sizeof(send_buf), 0)) == -1) {
            perror("send failde");
            exit(1);
        }
        recv_len = recv(socked_fd, recv_buf, sizeof(recv_buf), 0);
        recv_buf[recv_len] = '\0';
        if (recv_len == -1) {
            perror("faile to recv");
            exit(1);
        }
        getTime();
        fprintf(stdout, "\n[%s]:%s\n", time_str, recv_buf);
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