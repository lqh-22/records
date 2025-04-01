#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>			 // close()
#include <string.h>			 // strcpy, memset(), and memcpy()
#include <netdb.h>			 // struct addrinfo
#include <sys/types.h>		 // needed for socket(), uint8_t, uint16_t, uint32_t
#include <sys/socket.h>		 // needed for socket()
#include <netinet/in.h>		 // IPPROTO_ICMP, INET_ADDRSTRLEN
#include <netinet/ip.h>		 // struct ip and IP_MAXPACKET (which is 65535)
#include <netinet/ip_icmp.h> // struct icmp, ICMP_ECHO
#include <arpa/inet.h>		 // inet_pton() and inet_ntop()
#include <sys/ioctl.h>		 // macro ioctl is defined
#include <asm/ioctls.h>	 // defines values for argument "request" of ioctl.
#include <net/if.h>			 // struct ifreq
//#include <linux/if_ether.h>	 // ETH_P_IP = 0x0800, ETH_P_IPV6 = 0x86DD
#include <linux/if_packet.h> // struct sockaddr_ll (see man 7 packet)
#include <net/ethernet.h>
#include <math.h>
#include <errno.h>		  // errno, perror()
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <time.h>
#define ETH_P_DEAN 0x8100 // IEEE 802.1Q

int timestamp_t1s, timestamp_t1ns, timestamp_t2s, timestamp_t2ns, timestamp_t3s, timestamp_t3ns, timestamp_t4s, timestamp_t4ns, timestamp_tnows, timestamp_tnowns;
int read_s, read_ns, offset_s, offset_ns, offset, temp1, temp2;
struct timespec time_t1={0, 0},time_t2={0, 0},time_t3={0, 0},time_t4={0, 0},time_now={0, 0},time_adj={0, 0};

int main(int argc, char **argv)
{
	int ret;
	int sockfd_r;
	struct timeval timeout;
	fd_set fds;
	struct sockaddr_in client_addr;
	unsigned char recv_buffer[1024];
	socklen_t len;
	float last_v = 0, last_D = 0, last_id = 0;
	
	int flag,i;

	struct ifreq ifr = {0};
	struct sockaddr_can can_addr = {0};
	struct can_frame frame = {0};
	int sockfd = -1;


	sockfd_r = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_DEAN)); //接收套接字
	if (sockfd_r < 0)
	{
		printf("create sockfd_r failed\n");
	}
	else
	{
		while (1)
		{
			FD_ZERO(&fds);
			FD_SET(sockfd_r, &fds);
			timeout.tv_sec = 2;
			timeout.tv_usec = 0;

			memset(recv_buffer, 0, 1024);
			len = sizeof(client_addr);

			ret = select(sockfd_r + 1, &fds, NULL, NULL, &timeout);

			if (ret == 0)
			{
				// zlog_info(zlogc,"select time out#######continue\n");
				continue;
			}
			else if (ret == -1)
			{
				// zlog_info(zlogc,"select error");
				continue;
			}
			else
			{
				if (FD_ISSET(sockfd_r, &fds))
				{
					int n = 0,can_num=0;
					if ((n = recvfrom(sockfd_r, recv_buffer, 1024, 0, (struct sockaddr *)&client_addr, &len)) < 0)
					{
						continue;
					}
					clock_gettime(CLOCK_REALTIME, &time_t2);
            				// 记录t2时间
            				//timestamp_t2s = *((volatile u32 *)read_regAddr_s);
            				//timestamp_t2ns = *((volatile u32 *)read_regAddr_ns);
            				timestamp_t2s = time_t2.tv_sec;
            				timestamp_t2ns = time_t2.tv_nsec;

            				//timestamp_t1s = (recv_buffer[0] << 24) + (recv_buffer[1] << 16) + (recv_buffer[2] << 8) + recv_buffer[3];
            				//timestamp_t1ns = (recv_buffer[4] << 24) + (recv_buffer[5] << 16) + (recv_buffer[6] << 8) + recv_buffer[7];
					
					timestamp_t1s = (recv_buffer[21] << 24) + (recv_buffer[20] << 16) + (recv_buffer[19] << 8) + recv_buffer[18];
            				timestamp_t1ns = (recv_buffer[25] << 24) + (recv_buffer[24] << 16) + (recv_buffer[23] << 8) + recv_buffer[22];

					offset_s = timestamp_t2s - timestamp_t1s;
            				offset_ns = timestamp_t2ns - timestamp_t1ns;
					offset = offset_s * 1000000 + offset_ns;
					printf("end-to-end delay:%d\n",offset);
					printf("t1:%ds %dns\n",timestamp_t1s,timestamp_t1ns);
					printf("t2:%ds %dns\n",timestamp_t2s,timestamp_t2ns);
					
					//printf("\n recv TSN frame (%d Byte):[\n", n);
					//for (i = 0; i < n; i++)
					//{
					//	printf("%02x ", recv_buffer[i]);
					//}
					//printf("]\n");

				}
			}
		}
	}
	close(sockfd_r);
}
