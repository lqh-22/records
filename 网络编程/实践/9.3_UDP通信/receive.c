#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char const *argv[]){
    // 格式判断
    if (argc != 3){
        fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    int socket_fd;
    struct sockaddr_in serveraddr, clinetaddr;
    char recv_buf[32] = "", send_buf[32] = "";
    socklen_t addrlen = sizeof(serveraddr);

    // 创建socket
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("faile to build socket");
        exit(1);
    }
    // 填充服务器网络数据结构
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
    serveraddr.sin_port = htons(atoi(argv[2]));
    // 绑定socket
    if ((bind(socket_fd, (struct sockaddr *)&serveraddr, addrlen)) < 0){
        perror("faile to bind");
        exit(1);
    }
    // 开始通信
    while (1){
        if (recvfrom(socket_fd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&clinetaddr, &addrlen) < 0){
            perror("faile to recvfrom");
            exit(1);
        }
        fprintf(stdout, "[%s - %d]:%s\n", inet_ntoa(clinetaddr.sin_addr), ntohs(clinetaddr.sin_port), recv_buf);

        // 将受到的数据进行处理，再发送给客户端
        strcpy(send_buf, recv_buf);
        strcat(send_buf, "*_*");
        if (sendto(socket_fd, send_buf, sizeof(send_buf), 0, (struct sockaddr *)&clinetaddr, addrlen) < 0){
            perror("fail to sendto");
            exit(1);
        }
    }
    close(socket_fd);
    return 0;
    
}