#include <stdio.h>
#include "unistd.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "string.h"
#include "stdlib.h"

#define IP "192.168.0.61"
#define IP_dst "192.168.0.141"
#define PORT 8888
int fb = 0, len = 0;
struct sockaddr_in List_buf, from;
char buffer[512] = "I am here";
// int send_udp_data(){
// //发送数据
//         // bzero(buf,512);
//         printf("要发送的数据>>\r\n");
//         sendto(fb,buf,strlen(buf),0,(struct sockaddr *)&from,len);
// }
int udp_init()
{

    //创建socket编程
    int serfd = 0;
    serfd = socket(AF_INET, SOCK_DGRAM, 0); // creat socket
    if (serfd < 0)
    {
        perror("sockert failed");
        return -1;
    }
    // bind ip address and port
    int ret = 0;
    struct sockaddr_in serveraddr = {0};
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serveraddr.sin_port = htons(PORT);
    ret = bind(serfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (ret < 0)
    {
        perror("bind failed");
        close(serfd);
    }
    printf("bind success\r\n");

    struct sockaddr_in clientaddr = {0};
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    clientaddr.sin_port = htons(PORT);
    int client_Addr_len = sizeof(clientaddr);
    while (1)
    {
        ret = sendto(serfd, buffer, strlen(buffer), 0, (struct sockaddr *)&clientaddr, client_Addr_len);
        if (ret < 0)
        {
            perror("sendto failed");
            close(serfd);
            return -1;
        }
        
        printf("sendto successfully\r\n");
        sleep(1);
    }
}
