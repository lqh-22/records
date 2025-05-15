#include <stdio.h>      // 标准输入输出库，用于文件操作、输入输出等功能
#include <stdlib.h>     // 标准库，用于内存分配、进程控制等功能
#include <string.h>     // 字符串处理库，用于字符串操作函数
#include <time.h>      // 时间处理库，用于获取当前时间
#include <winsock2.h>  // Windows Socket API，用于网络编程

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
 * 断点续传用，需要保存到flash中
 * 1. 断点续传的文件名（分区名/固件名）
 * 2. 断点续传的总文件大小
 * 3. 已经传输的文件大小
 * 4. 标志位，完整传输后值为1，未完成传输值为0
 * 5. 版本号
 */
typedef struct {
    char fileName[256]; // 文件名
    int fileSize; // 文件字节大小
    int writeBytes; // 已经传输的文件大小
    int flag; // 标志位，完整传输后值为1，未完成传输值为0
    char version[16]; // 版本号
} flashData; // flash数据包结构体

/**
 * 客户端发送数据起始字节，请求下一字节大小
 */
typedef struct {
    int sign;
    int startBytes; // 起始字节
    int reqBytes; // 请求下一字节大小
} reqDataSize;

char recvBuf[MAXSIZE], sendBuf[MAXSIZE];  // 接受和发送缓冲区
FILE *fileFp, *otaFp;  // 执行文件和otaFlash文件指针
headData reqHead, resHead; // 请求头和响应头数据包结构体;
flashData otaFlash; // flash数据包结构体
reqDataSize reqData; // 请求数据包结构体
int fileSize = 0, writeBytes = 0; // 文件大小和已经传输的文件大小

int main(int argc, char const *argv[]){
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <ip> <port> <filename>\n", argv[0]);
        exit(1);
    }
    // win下的socket编程需要初始化winsock
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa); // 初始化Winsock
    /**
     * 1.sock
     * 2.connect
     * 3.send
     */
    SOCKET remoteFd;
    struct sockaddr_in sockServer;
    remoteFd = socket(AF_INET, SOCK_STREAM, 0);
    if (remoteFd == -1) {
        printf("fail to create socket\n");
        exit(1);
    }
    sockServer.sin_family = AF_INET;
    sockServer.sin_addr.s_addr = inet_addr(argv[1]);
    sockServer.sin_port = htons(atoi(argv[2]));
    memset(&(sockServer.sin_zero), 0, 8); // 清零结构体的剩余部分

    if (connect(remoteFd, (struct sockaddr *)&sockServer, sizeof(struct sockaddr)) == -1) {
        fprintf(stderr, "fail to connect\n");
        exit(1);
    }
    // 发送头数据包
    reqHead.sign = SIGN;
    strcpy(reqHead.fileName, argv[3]);
    memset(sendBuf, 0x00, sizeof(sendBuf));
    memcpy(sendBuf, &reqHead, sizeof(headData));
    if ((send(remoteFd, sendBuf, sizeof(reqHead), 0)) == -1) {
        fprintf(stderr, "fail to send\n");
        exit(1);
    }
    /**
     * 接受服务器发送的响应头数据包
     * 1. 初始化flash数据包结构体，存到flash中
     */
    int recvSize = recv(remoteFd, recvBuf, sizeof(headData), 0);
    if (recvSize == -1) {
        printf("fail to recv\n");
        WSACleanup(); // 清理Winsock
        exit(1);
    }
    memcpy(&resHead, recvBuf, sizeof(headData)); // 将接收到的数据拷贝到头数据包结构体中
    if (resHead.sign != SIGN) {
        // 签名有问题，需要重新接受文件
        printf("sign error\n");
        return 0;
    } else {
        // otaFlashInfo.txt中是否有数据，若有数据则根据文件名、版本号及flag值判断是否断点续传
        // w会清空文件内容，wb+会清空文件内容并创建新文件
        otaFp = fopen("otaFlashInfo.txt", "rb+");
        if (otaFp == NULL) {
            // 如果文件不存在，则创建一个新文件
            otaFp = fopen("otaFlashInfo.txt", "wb+");
        }
        int readNum = fread(&otaFlash, sizeof(flashData), 1, otaFp);
        if (readNum == 1 && otaFlash.flag == 0 && strcmp(otaFlash.fileName, resHead.fileName) == 0 && strcmp(otaFlash.version, resHead.version) == 0) {
            // 断点续传
            printf("continue to recv file %s\n", otaFlash.fileName);
        } else {
            // 新文件传输
            otaFlash.fileSize = resHead.fileSize; // 文件字节大小
            otaFlash.writeBytes = 0; // 已经传输的文件大小
            otaFlash.flag = 0; // 标志位，完整传输后值为1，未完成传输值为0
            strcpy(otaFlash.fileName, resHead.fileName); // 文件名
            strcpy(otaFlash.version, resHead.version); // 版本号
            fseek(otaFp, 0, SEEK_SET);
            fwrite(&otaFlash, sizeof(flashData), 1, otaFp); // 将数据包结构体写入文件中
        }
        fileSize = otaFlash.fileSize;
        writeBytes = otaFlash.writeBytes;
    }

    /**
     * 开始接受固件
     * 0. 发送请求数据报文
     *      用结构体代替
     * 1. 接受数据
     * 2. 写入文件
     * 3. 更新otaFlashInfo.txt中的数据
     */
    // 要断点续传，要以ab+的方式打开文件，ab+会在文件末尾追加数据
    fileFp = fopen(resHead.fileName, "ab+");
    while (writeBytes < fileSize){
        printf("\rFinished:%.2f%% total %d bytes, recved %d bytes", writeBytes * 1.0 / fileSize * 100, fileSize, writeBytes);
        reqData.sign = SIGN;
        reqData.startBytes = writeBytes;
        if (fileSize - writeBytes > MAXSIZE) {
            reqData.reqBytes = MAXSIZE;
        } else {
            reqData.reqBytes = fileSize - writeBytes;
        }
        memset(sendBuf, 0x00, sizeof(sendBuf));
        memcpy(sendBuf, &reqData, sizeof(reqData));
        if ((send(remoteFd, sendBuf, sizeof(reqData), 0)) == -1) {
            fprintf(stderr, "fail to send\n");
            exit(1);
        }
        if ((recvSize = recv(remoteFd, recvBuf, MAXSIZE, 0)) == -1) {
            printf("fail to recv file\n");
            WSACleanup(); // 清理Winsock
            exit(1);
        }
        // 将文件指针移动到指定的位置开始写入数据
        fseek(fileFp, reqData.startBytes, SEEK_SET);
        fwrite(recvBuf, sizeof(char), reqData.reqBytes, fileFp);
        writeBytes += reqData.reqBytes;
        otaFlash.writeBytes = writeBytes; // 更新已经传输的文件大小
        if (writeBytes == fileSize) otaFlash.flag = 1;  // 更新标志位，完整传输后值为1，未完成传输值为0
        fseek(otaFp, 0, SEEK_SET);
        fwrite(&otaFlash, sizeof(flashData), 1, otaFp); // 将数据包结构体写入文件中
    }
    printf("\rFinished:%.2f%% total %d bytes, recved %d bytes", writeBytes * 1.0 / fileSize * 100, fileSize, writeBytes);
    // 发送exit标记，表示传输完成
    reqData.sign = EXIT;
    memset(sendBuf, 0x00, sizeof(sendBuf));
    memcpy(sendBuf, &reqData, sizeof(reqData));
    if ((send(remoteFd, sendBuf, sizeof(reqData), 0)) == -1) {
        fprintf(stderr, "fail to send\n");
        exit(1);
    }
    
    printf("\nrecv file %s success, total %d bytes\n", reqHead.fileName, resHead.fileSize);
    fclose(fileFp);
    fclose(otaFp);
    WSACleanup(); // 清理Winsock









    return 0;
}