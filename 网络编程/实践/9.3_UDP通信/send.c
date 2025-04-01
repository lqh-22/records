#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char const *argv[]){
    // 格式判断
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }
    int sockfd;
    struct sockaddr_in serveraddr;
    socklen_t addrlen = sizeof(serveraddr);
    char send_buf[32] = "", recv_buf[32] = "";

    // 创建套接字
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("fail to build socket");
        exit(1);
    }

    // 填充服务器网络信息结构体
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
    serveraddr.sin_port = htons(atoi(argv[2]));

    // 与服务器通信
    while (1){
        // 填充要发送数据缓存
        fgets(send_buf, sizeof(send_buf), stdin);
        // fgets会将换行符读入，将最后的换行符替代为\0
        send_buf[strlen(send_buf) - 1] = '\0';
        if (sendto(sockfd, send_buf, sizeof(send_buf), 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0){
            perror("fail to sendto");
            exit(1);
        }

        // 接受服务器发送过来的数据
        if (recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&serveraddr, &addrlen) < 0){
            perror("fail to recvfrom");
            exit(1);
        }
        fprintf(stdout, "from server:%s\n", recv_buf);
    }
    close(sockfd);
    return 0;
}   