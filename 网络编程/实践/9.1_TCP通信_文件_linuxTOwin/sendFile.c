#include <stdio.h>      // 标准输入输出库，用于文件操作、输入输出等功能
#include <stdlib.h>     // 标准库，提供内存分配、进程控制、随机数生成等功能
#include <string.h>     // 字符串操作库，提供字符串处理函数，如strlen, strcpy等
#include <netinet/in.h> // 定义Internet地址族相关的结构和常量，如sockaddr_in结构
#include <sys/socket.h> // 提供套接字编程的基本接口，如socket, bind, listen, accept等
#include <sys/types.h>  // 定义数据类型，用于与系统调用相关的操作
#include <netdb.h>      // 提供与网络数据库相关的操作，如主机名和IP地址的解析
#include <arpa/inet.h>  // 提供 inet_addr 函数的声明
#include <unistd.h>  // 提供 close 函数的声明
#include <time.h>  // 提供 time 函数的声明

char *getTime();  // 函数声明，用于获取当前时间的字符串

#define SIGN 0xAABBCCDD // 检验标记
#define MAXSIZE 1024 // 最大数据包大小

typedef struct {
    int sign; // 检验标记
    int fileSize; // 文件字节大小
    char fileName[256]; // 文件名
} headData; // 头数据包结构体

char sendBuf[MAXSIZE];  // 发送缓冲区
char time_str[64];  // 存储目前时间

/**
 * 从文件中读取二进制数据发送
 */
int main(int argc, char const *argv[]) {
    if(argc != 4) {
        fprintf(stderr, "Usage: %s <ip> <port> <filename>\n", argv[0]);
        exit(1);
    }
    /**
     * 1.sock
     * 2.connect
     * 3.send
     */
    int sockFd;
    struct sockaddr_in sockServer;
    sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd == -1) {
        fprintf(stderr, "[%s] fail to create socket\n", getTime());
        exit(1);
    }
    sockServer.sin_family = AF_INET;
    sockServer.sin_addr.s_addr = inet_addr(argv[1]);
    sockServer.sin_port = htons(atoi(argv[2]));
    memset(&(sockServer.sin_zero), 0, 8); // 清零结构体的剩余部分
    if (connect(sockFd, (struct sockaddr *)&sockServer, sizeof(struct sockaddr)) == -1) {
        fprintf(stderr, "[%s] fail to connect\n", getTime());
        exit(1);
    }
    /**
     * 初始化头数据包
     *  1.检验标记
     *  2.文件字节大小
     *  3.文件名
     * 将头数据通过memcpy拷贝到字符串数组发送到服务器，服务器端接收后，使用memcpy将数据拷贝到头数据包结构体中
     */
    headData head;
    head.sign = SIGN;
    strcpy(head.fileName, argv[3]);
    FILE *fp = fopen(argv[3], "rb+");
    fseek(fp, 0, SEEK_END);
    head.fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    // 通过memcpy将头数据拷贝到发送缓冲区, 需要注意先通过memcpy将发送缓冲区清零
    memset(sendBuf, 0, sizeof(sendBuf));
    memcpy(sendBuf, &head, sizeof(head));
    printf("file size: %d\n", head.fileSize);
    if ((send(sockFd, sendBuf, sizeof(head), 0)) == -1) {
        fprintf(stderr, "[%s] fail to send\n", getTime());
        exit(1);
    }
    /**
     * 发送文件的二进制数据
     * 1.每次读取MAXSIZE大小的数据
     * 2.通过send发送数据实际读取的字节大小
     * 3.通过feof判断文件是否读取完毕
     */
    int readBytes = 0;
    while (!feof(fp)) {
        readBytes = fread(sendBuf, sizeof(char), MAXSIZE, fp);
        if (readBytes == -1) {
            fprintf(stderr, "[%s] fail to read file\n", getTime());
            exit(1);
        }
        if ((send(sockFd, sendBuf, readBytes, 0)) == -1) {
            fprintf(stderr, "[%s] fail to send file\n", getTime());
            exit(1);
        }
    }
    /**
     * 关闭套接字、文件描述符
     */
    close(sockFd);
    fclose(fp);
    fprintf(stdout, "[%s] file %d size %s send success\n", getTime(), head.fileSize, argv[3]);

    return 0;
}

char *getTime() {
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
    return time_str;
}