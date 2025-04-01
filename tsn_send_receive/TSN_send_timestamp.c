 #include <stdio.h>
#include <stdlib.h>
#include <unistd.h>          // close()
#include <string.h>          // strcpy, memset(), and memcpy()
#include <netdb.h>           // struct addrinfo
#include <sys/types.h>       // needed for socket(), uint8_t, uint16_t, uint32_t
#include <sys/socket.h>      // needed for socket()
#include <netinet/in.h>      // IPPROTO_ICMP, INET_ADDRSTRLEN
#include <netinet/ip.h>      // struct ip and IP_MAXPACKET (which is 65535)
#include <netinet/ip_icmp.h> // struct icmp, ICMP_ECHO
#include <arpa/inet.h>       // inet_pton() and inet_ntop()
#include <sys/ioctl.h>       // macro ioctl is defined
#include <asm/ioctls.h>     // defines values for argument "request" of ioctl.
#include <net/if.h>          // struct ifreq
//#include <linux/if_ether.h>  // ETH_P_IP = 0x0800, ETH_P_IPV6 = 0x86DD
#include <linux/if_packet.h> // struct sockaddr_ll (see man 7 packet)
#include <net/ethernet.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <time.h>

#include <errno.h>        // errno, perror()
#define ETH_P_DEAN 0x8100 // IEEE 802.1Q
#define CAN_MESSAGE 5

int timestamp_t1s, timestamp_t1ns, timestamp_t2s, timestamp_t2ns, timestamp_t3s, timestamp_t3ns, timestamp_t4s, timestamp_t4ns, timestamp_tnows, timestamp_tnowns;
int read_s, read_ns, offset_s, offset_ns, offset, temp1, temp2;
struct timespec time_t1={0, 0},time_t2={0, 0},time_t3={0, 0},time_t4={0, 0},time_now={0, 0},time_adj={0, 0};

int main(int argc, char **argv)
{
    int i, datalen, frame_length, sd, bytes;
    uint8_t *interface = "enp5s0";
    uint8_t data[IP_MAXPACKET];
    uint8_t src_mac[6];
    uint8_t dst_mac[6];
    ;
    // uint8_t ether_frame[IP_MAXPACKET];
    uint8_t ether_frame[IP_MAXPACKET];
    struct sockaddr_ll device;
    struct ifreq ifr;
    char buf_can[512], buf[512], temp[32];
    struct sockaddr_can can_addr = {0};
    struct can_frame frame = {0};
    int sockfd_can = -1;
    int ret_can;
    uint8_t priority[8] = {0xe0,0xc0,0xa0,0x80,0x60,0x40,0x20,0x00};
    srand(time(NULL));    

 
    // Submit request for a socket descriptor to look up interface.
    if ((sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
    { //第一次创建socket是为了获取本地网卡信息
        perror("socket() failed to get socket descriptor for using ioctl() ");
        exit(EXIT_FAILURE);
    }

    // Use ioctl() to look up interface name and get its MAC address.
    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", interface);
    if (ioctl(sd, SIOCGIFHWADDR, &ifr) < 0)
    {
        perror("ioctl() failed to get source MAC address ");
        return (EXIT_FAILURE);
    }
    close(sd);

    // Copy source MAC address.
    memcpy(src_mac, ifr.ifr_hwaddr.sa_data, 6);

    // Report source MAC address to stdout.
    printf("MAC address for interface %s is ", interface);
    for (i = 0; i < 5; i++)
    {
        printf("%02x:", src_mac[i]);
    }
    printf("%02x\n", src_mac[5]);

    // Find interface index from interface name and store index in
    // struct sockaddr_ll device, which will be used as an argument of sendto().
    memset(&device, 0, sizeof(device));
    if ((device.sll_ifindex = if_nametoindex(interface)) == 0)
    {
        perror("if_nametoindex() failed to obtain interface index ");
        exit(EXIT_FAILURE);
    }
    printf("Index for interface %s is %i\n", interface, device.sll_ifindex);

    memset(buf, 0x0, sizeof(buf));

    if ((sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_DEAN))) < 0)
    { //创建正真发送的socket
        perror("socket() failed ");
        exit(EXIT_FAILURE);
    }
    int can_message = 0, can_byte = 0; //记录CAN消息数量
    // 98:0e:24:be:d6:fc
    // b4:96:91:55:c8:1d
    // dst_mac[0] = 0xb4; //设置目的网卡地址 eth2
    // dst_mac[1] = 0x96;
    // dst_mac[2] = 0x91;
    // dst_mac[3] = 0x55;
    // dst_mac[4] = 0xc8;
    // dst_mac[5] = 0x1d;

    dst_mac[0] = 0x98; //设置目的网卡地址
    dst_mac[1] = 0x0e;
    dst_mac[2] = 0x24;
    dst_mac[3] = 0xb8;
    dst_mac[4] = 0xd6;
    dst_mac[5] = 0xfc;

    // Fill out sockaddr_ll.
    device.sll_family = AF_PACKET;
    memcpy(device.sll_addr, src_mac, 6);
    device.sll_halen = htons(6);

    memcpy(ether_frame, dst_mac, 6);
    memcpy(ether_frame + 6, src_mac, 6);
    /*
    uint8_t tsn_tag[10];
    tsn_tag[0] = 0x81;
    tsn_tag[1] = 0x00;
    //tsn_tag[2] = 0xe0; //priority = 5
    //tsn_tag[3] = 0x0a; //vlan id = 10
    //tsn_tag[4] = ETH_P_DEAN / 256;
    //tsn_tag[5] = ETH_P_DEAN % 256;
    */
    ether_frame[12] = 0x81;
    ether_frame[13] = 0x00;
    //ether_frame[14] = 0xe0; // priority = 7
    //ether_frame[14] = priority[]; // priority = 7
    // 有bug,0000 0000 左前三位为vid,第四位为其他字段，后四位为优先级
    ether_frame[15] = 0x0a; // vlan id = 10
    ether_frame[16] = ETH_P_DEAN / 256;
    ether_frame[17] = ETH_P_DEAN % 256;

    // memcpy (ether_frame + 2, tsn_tag, 2);

    for (i=0;i<100;i++)
    {
	int f = rand()%8;
	ether_frame[14] = priority[f];
        // 发送的data，长度可以任意，但是抓包时看到最小数据长度为46，这是以太网协议规定以太网帧数据域部分最小为46字节，不足的自动补零处理
        data[0] = priority[f]; // priority = 5
        data[1] = 0x0a; // vlan id = 10
        data[2] = ETH_P_DEAN / 256;
        data[3] = ETH_P_DEAN % 256;
        clock_gettime(CLOCK_REALTIME, &time_t1);
        timestamp_t1s = time_t1.tv_sec;
        timestamp_t1ns = time_t1.tv_nsec;

        memcpy(data+4, &timestamp_t1s, sizeof(timestamp_t1s));
        memcpy(data+ 4 + sizeof(timestamp_t1s), &timestamp_t1ns, sizeof(timestamp_t1ns));

        // Fill out ethernet frame header.
        // frame_length = 6 + 6 + 6   + datalen;
        // Destination and Source MAC addresses

        // data
        datalen = 4 + sizeof(timestamp_t1s) + sizeof(timestamp_t1ns);
        frame_length = 18 + datalen;
        memcpy(ether_frame + 18, data, datalen);

        // Submit request for a raw socket descriptor.

        // Send ethernet frame to socket.
        if ((bytes = sendto(sd, ether_frame, frame_length, 0, (struct sockaddr *)&device, sizeof(device))) <= 0)
        {
            perror("sendto() failed");
            exit(EXIT_FAILURE);
        }
        printf("send num=%d\n", frame_length);
	printf("t1:%ds %dns\n",timestamp_t1s,timestamp_t1ns);
	
	usleep(10000);
    }

    // Close socket descriptor.
    close(sd);
    close(sockfd_can);
    return (EXIT_SUCCESS);
}


