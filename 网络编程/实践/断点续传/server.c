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
#define EXIT 0xFFFFFFFF // 退出标记
#define MAXSIZE 1024 // 最大数据包大小

typedef struct {
    int sign; // 检验标记
    int fileSize; // 文件字节大小
    char fileName[256]; // 文件名
    char version[16]; // 版本号
} headData; // 头数据包结构体

/**
 * 客户端发送数据起始字节，请求下一字节大小
 */
typedef struct {
    int sign;
    int startBytes; // 起始字节
    int reqBytes; // 请求下一字节大小
} reqDataSize;

char sendBuf[MAXSIZE], recvBuf[MAXSIZE];
char time_str[64];  // 存储目前时间
headData resHead;
reqDataSize reqSize;
FILE *fp;

/**
 * 从文件中读取二进制数据发送
 */
int main(int argc, char const *argv[]) {
    if(argc != 3) {
        fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }
    /**
     * 1.sock
     * 2.bind
     * 3.listen
     * 3.accept
     * 4.send
     */
    int sockFd, remoteFd;
    struct sockaddr_in sockServer, sockRemote;
    sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd == -1) {
        printf("fail to create socket\n");
        exit(1);
    }
    sockServer.sin_family = AF_INET;
    sockServer.sin_addr.s_addr = inet_addr(argv[1]);
    sockServer.sin_port = htons(atoi(argv[2]));
    memset(&(sockServer.sin_zero), 0, 8); // 清零结构体的剩余部分
    if (bind(sockFd, (struct sockaddr *)&sockServer, sizeof(sockServer)) == -1) {
        printf("fail to bind\n");
        exit(1);
    }
    if (listen(sockFd, 10) == -1) {
        printf("fail to listen\n");
        exit(1);
    }
    while (1) {
        socklen_t addrLen = sizeof(struct sockaddr_in);
        printf("waiting for client...\n");
        remoteFd = accept(sockFd, (struct sockaddr *)&sockRemote, &addrLen);
        if (remoteFd == -1) {
            printf("fail to accept\n");
            exit(1);
        }
        // 对于每一个TCP连接，都要打印发送端的IP和端口
        printf("RemoteIP: %s RemotePort: %d\n", inet_ntoa(sockRemote.sin_addr), ntohs(sockRemote.sin_port));
        /**
         * 1.接收头数据包
         *      1.检验标记
         *      2.初始化头数据包
         * 2.接收文件数据
         * 3.保存文件
         */
        int recvSize = recv(remoteFd, recvBuf, sizeof(headData), 0);
        if (recvSize == -1) {
            printf("fail to recv\n");
            exit(1);
        }
        memcpy(&resHead, recvBuf, sizeof(headData)); // 将接收到的数据拷贝到头数据包结构体中
        if (resHead.sign != SIGN) {
            // 签名有问题，需要重新接受文件
            printf("sign error\n");
        } else {
            // 签名没有问题，补充头数据包发送
            strcpy(resHead.version, "v1.0");
            fp = fopen(resHead.fileName, "rb+");
            fseek(fp, 0, SEEK_END);
            resHead.fileSize = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            // 通过memcpy将头数据拷贝到发送缓冲区, 需要注意先通过memcpy将发送缓冲区清零
            memset(sendBuf, 0, sizeof(sendBuf));
            memcpy(sendBuf, &resHead, sizeof(headData));
            printf("file name: %s, file size: %d\n", resHead.fileName, resHead.fileSize);
            if ((send(remoteFd, sendBuf, sizeof(headData), 0)) == -1) {
                fprintf(stderr, "[%s] fail to send\n", getTime());
                exit(1);
            }
        }
    
        /**
         * 发送固件
         * 1.接受需要发送固件起始地址和大小
         * 2.发送固件字节数据
         */
        while (1) {
            if ((recvSize = recv(remoteFd, recvBuf, MAXSIZE, 0)) == -1) {
                printf("exit\n");
                break;
            }
            memcpy(&reqSize, recvBuf, sizeof(reqDataSize));
            if(reqSize.sign != SIGN && reqSize.sign != EXIT) {
                // 签名有问题，需要重新接受文件
                printf("sign error\n");
                return 0;
            } else if (reqSize.sign == EXIT) {
                // 退出标记，退出循环
                fprintf(stdout, "[%s] file %s size %d send success\n", getTime(), resHead.fileName, resHead.fileSize);
                break;
            }
            // 发送固件字节
            fseek(fp, reqSize.startBytes, SEEK_SET); // 设置文件指针位置
            fread(sendBuf, sizeof(char), reqSize.reqBytes, fp);
            if ((send(remoteFd, sendBuf, reqSize.reqBytes, 0)) == -1) {
                fprintf(stderr, "[%s] fail to send file\n", getTime());
                exit(1);
            }
        }
    }
    
    /**
     * 关闭套接字、文件描述符
     */
    close(sockFd);
    close(remoteFd);
    fclose(fp);

    return 0;
}

char *getTime() {
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
    return time_str;
}