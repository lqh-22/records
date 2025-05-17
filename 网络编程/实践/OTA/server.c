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
#include <dirent.h>  // 目录操作库，用于遍历目录

#include "bsdiff/bsdiff.h" // bsdiff相关函数的头文件

// 用于判断目录项类型
#ifndef DT_DIR
#define DT_DIR 4
#endif

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

/**
 * 服务端：存放固件版本列表
 */
typedef struct {
    char fileName[256];  // 固件名
    char version[16]; // 版本号
} verTable; // 版本号表结构体

#define VER_TABLE_COUNT 20 // 版本号个数

char sendBuf[MAXSIZE], recvBuf[MAXSIZE];
char time_str[64];  // 存储目前时间
headData resHead;
reqDataSize reqSize;
FILE *fp;

void readVerTable(verTable *table);

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
            /**
             * 0.找本地最新固件与版本号
             * 1.根据接收到的文件名，版本号生成补丁包
             * 2.填充头数据包文件名、版本号（最新的版本号）
             */
            verTable table[VER_TABLE_COUNT];
            verTable *verTableG = NULL;
            // 读取本地固件名和最新版本号存到table中
            // printf("@\n");
            readVerTable(table);
            // printf("@0\n");
            // 提取test.bin中的test和bin
            char baseFileName[256], baseFileType[256];
            sscanf(resHead.fileName, "%255[^.].%255s", baseFileName, baseFileType);
            for (int i = 0; i < VER_TABLE_COUNT; ++i) {
                if (strcmp(table[i].fileName, baseFileName) == 0) {
                    verTableG = &(table[i]); // 读取版本号表
                    break;
                }
            }
            // printf("@1\n");
            if (verTableG == NULL) {
                printf("file %s not found\n", resHead.fileName);
                continue;
            }
            char oldFileName[256], newFileName[256], patchFileName[256];
            snprintf(oldFileName, sizeof(oldFileName), "./app/%s/%s/%s", baseFileName, resHead.version, resHead.fileName);
            snprintf(newFileName, sizeof(newFileName), "./app/%s/%s/%s", baseFileName, verTableG->version, resHead.fileName);
            snprintf(patchFileName, sizeof(patchFileName), "./app/patch/%s_%s_to_%s_patch", baseFileName, resHead.version, verTableG->version);
            // 在patchFileName下生成补丁包
            printf("old file: %s, new file: %s, patch file: %s\n", oldFileName, newFileName, patchFileName);
            bsdiffFile(oldFileName, newFileName, patchFileName);
            // 发送补丁包
            sscanf(patchFileName, "./app/patch/%255s", resHead.fileName); // 文件名
            // strcpy(resHead.fileName, "patch.txt"); // 文件名
            strcpy(resHead.version, verTableG->version); // 版本号
            fp = fopen(patchFileName, "rb+");
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

int compare_version(const char *v1, const char *v2) {
    if (v2[0] == '\0') return 1;
    int v1_major, v1_minor;
    int v2_major, v2_minor;
    // 假设格式为 vX.Y
    sscanf(v1, "v%d.%d", &v1_major, &v1_minor);
    sscanf(v2, "v%d.%d", &v2_major, &v2_minor);
    if (v1_major != v2_major)
        return v1_major > v2_major ? 1 : -1;
    if (v1_minor != v2_minor)
        return v1_minor > v2_minor ? 1 : -1;
    return 0;
}

void readVerTable(verTable *table) {
    DIR *dir;
    struct dirent *entry;
    int i = 0;

    // 打开目录
    dir = opendir("app");
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    // 遍历目录中的文件
    /**
     * 扫描app文件夹
     * 1. 读取除了patch文件夹以外的所有文件夹
     * 2. 对读到的文件夹，记录文件夹名
     * 3. 遍历读到的文件夹，记录vx.x的最大值
     */
    while ((entry = readdir(dir)) != NULL) {
        // 排除当前目录和上级目录
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, "patch") == 0) {
            continue;
        }
        // 判断是否为目录
        // printf("entry name: %s\n", entry->d_name);
        char dirName[256];
        sprintf(dirName, "app/%s", entry->d_name);
        // printf("dirName: %s\n", dirName);
        if (entry->d_type == DT_DIR) {
            // 读取文件夹名
            strcpy(table[i].fileName, entry->d_name);
            table[i].version[0] = '\0'; // 初始化版本号为空字符串
            // 遍历该目录下的文件，使用compare_version函数比较版本号
            DIR *subdir;
            struct dirent *subentry;
            // printf("name:%s\n", strcat("app/", entry->d_name));
            subdir = opendir(dirName);
            if (subdir == NULL) {
                perror("opendir");
                continue;
            }
            // printf("dirName: %s\n", dirName);
            while ((subentry = readdir(subdir)) != NULL) {
                // printf("#1\n");
                // 排除当前目录和上级目录
                if (strcmp(subentry->d_name, ".") == 0 || strcmp(subentry->d_name, "..") == 0) {
                    continue;
                }
                // 判断是否为文件
                // printf("#2\n");
                // printf("version: %s\n", subentry->d_name);
                if (subentry->d_type == DT_DIR) {
                    // 比较版本号
                    // printf("version: %s\n", subentry->d_name);
                    if (compare_version(subentry->d_name, table[i].version) > 0) {
                        strcpy(table[i].version, subentry->d_name);
                    }
                }
            }
            ++i;
        }
    }

    closedir(dir);
}