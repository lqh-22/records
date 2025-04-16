#include <stdio.h>      // 标准输入输出库，用于文件操作、输入输出等功能
#include <stdlib.h>     // 标准库，用于内存分配、进程控制等功能
#include <string.h>     // 字符串处理库，用于字符串操作函数
#include <time.h>      // 时间处理库，用于获取当前时间
#include <winsock2.h>  // Windows Socket API，用于网络编程

#define SIGN 0xAABBCCDD // 检验标记
#define MAXSIZE 1024 // 最大数据包大小

typedef struct {
    int sign; // 检验标记
    int fileSize; // 文件字节大小
    char fileName[256]; // 文件名
} headData; // 头数据包结构体

char recvBuf[MAXSIZE];  // 接受缓冲区
FILE *fp; // 文件指针

/**
 * 从网络接受二进制数据，存到文件中
 */
int main(int argc, char const *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }
    /**
     * 1.sock,sockaddr_in
     * 2.bind
     * 3.listen
     * 4.accept
     * 5.recv
     */
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa); // 初始化Winsock

    SOCKET sockFd, remoteFd;
    struct sockaddr_in sockServer, sockRemote;
    sockFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockFd == -1) {
        printf("fail to create socket\n");
        exit(1);
    }
    sockServer.sin_family = AF_INET;
    sockServer.sin_addr.s_addr = inet_addr(argv[1]); // 绑定到所有可用的接口
    sockServer.sin_port = htons(atoi(argv[2]));
    memset(&(sockServer.sin_zero), 0, 8); // 清零结构体的剩余部分
    if (bind(sockFd, (struct sockaddr *)&sockServer, sizeof(sockServer)) == -1) {
        printf("fail to bind\n");
        WSACleanup(); // 清理Winsock
        exit(1);
    }
    // 通过getsockbyname函数得到本地的IP和端口
    // char hostName[256];
    // HOSTENT *hostent = NULL;
    // gethostname(hostName, sizeof(hostName));
    // hostent = gethostbyname(hostName);
    // if (hostent == NULL) {
    //     printf("fail to get host name\n");
    //     WSACleanup(); // 清理Winsock
    //     exit(1);
    // }
    // struct in_addr *hostAddr = (struct in_addr *)*(hostent->h_addr_list);
    printf("IP: %s Port: %s\n", argv[1], argv[2]);
    if (listen(sockFd, 10) == -1) {
        printf("fail to listen\n");
        WSACleanup(); // 清理Winsock
        exit(1);
    }
    int addrLen = sizeof(struct sockaddr_in);
    while (1) {
        printf("waiting for client...\n");
        remoteFd = accept(sockFd, (struct sockaddr *)&sockRemote, &addrLen);
        if (remoteFd == -1) {
            printf("fail to accept\n");
            WSACleanup(); // 清理Winsock
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
        headData head;
        int recvSize = recv(remoteFd, recvBuf, sizeof(headData), 0);
        if (recvSize == -1) {
            printf("fail to recv\n");
            WSACleanup(); // 清理Winsock
            exit(1);
        }
        memcpy(&head, recvBuf, sizeof(headData)); // 将接收到的数据拷贝到头数据包结构体中
        if (head.sign != SIGN) {
            // 签名有问题，需要重新接受文件
            printf("sign error\n");
        } else {
            // 签名没有问题
            fp = fopen(head.fileName, "wb+");
            int writeBytes = 0, remainBytes = head.fileSize;
            while (writeBytes < head.fileSize) {
                if ((recvSize = recv(remoteFd, recvBuf, MAXSIZE, 0)) == -1) {
                    printf("fail to recv file\n");
                    WSACleanup(); // 清理Winsock
                    exit(1);
                }
                if (remainBytes >= MAXSIZE) {
                    fwrite(recvBuf, sizeof(char), MAXSIZE, fp);
                    writeBytes += MAXSIZE;
                    remainBytes -= MAXSIZE;
                }else {
                    fwrite(recvBuf, sizeof(char), remainBytes, fp);
                    writeBytes += remainBytes;
                    remainBytes = 0;
                }
                printf("\rFinished:%.2f%% total %d bytes, recved %d bytes", writeBytes * 1.0 / head.fileSize * 100, head.fileSize, writeBytes);
            }
            printf("\nrecv file %s success, total %d bytes\n", head.fileName, head.fileSize);
            fclose(fp);
        }
    }
    WSACleanup(); // 清理Winsock
    return 0;
}