/* Client program Broken FTP */
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
void die(char *s)
{
    perror(s);
    exit(1);
}
typedef struct packet
{
    int seq_no;
    int size;
    char data[256];
    int lastpkt;
    int channel;
    int type;
} pkt;
typedef struct packet1
{
    int seq_no;
} pkt1;
int main(void)
{
    int sockfd1 = 0,sockfd2=0;
    pkt send_pkt;
    pkt rcv_ack;
    // unsigned char buff_offset[10]; // buffer to send the File offset value
    // unsigned char buff_command[2]; // buffer to send the Complete File (0) or Partial File Command (1).
    // int offset;                    // required to get the user input for offset in case of partial file command
    // int command;                   // required to get the user input for command
    memset(send_pkt.data, '0', sizeof(send_pkt.data));
    struct sockaddr_in serv_addr;
    struct timeval timeout;
    fd_set fds;
    /* Create a socket first */
    if ((sockfd1 = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }
    if ((sockfd2 = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }
    /* Initialize sockaddr_in data structure */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8888); // port
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    /* Attempt a connection */
    if (connect(sockfd1, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }
    if (connect(sockfd2, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }
    /* Create file where data will be stored */
    FILE *fp;
    fp = fopen("input.txt", "r");
    if (NULL == fp)
    {
        printf("Error opening file");
        return 1;
    }
    int seq=0;
    fgets(send_pkt.data,256,fp);
    send_pkt.size=strlen(send_pkt.data);
    send_pkt.seq_no=seq;
    send_pkt.type=0;
    send_pkt.channel=1;
    if(write(sockfd1,&send_pkt,sizeof(send_pkt))!=0)
        die("write()");
    seq+=send_pkt.size;
    fgets(send_pkt.data,256,fp);
    send_pkt.size=strlen(send_pkt.data);
    send_pkt.seq_no=seq;
    send_pkt.type=0;
    send_pkt.channel=2;
    if(write(sockfd2,&send_pkt,sizeof(send_pkt))!=0)
        die("write()");
    seq+=send_pkt.size;
    // Else { command = 0 then no need to send the value of offset }
    /* Receive data in chunks of 256 bytes */
    int flag = 0;
    int state = 0;
    int ready = 0;
    int drop_flag = 0;
    int nread = 0;
    int bytesReceived = 0;
    while (1)
    {
        /* First read file in chunks of 256 bytes */
        switch (state)
        {
        case 0:
        {
            if(read(sockfd1,&rcv_ack,sizeof(rcv_ack)))
            {
                state=rcv_ack.channel;
            }
            else if(read(sockfd2,&rcv_ack,sizeof(rcv_ack)))
            {
                state=rcv_ack.channel;
            }
            break;
        }
        case 1:
        {
            if(feof(fp))
            {
                flag=1;
                break;
            }
            fgets(send_pkt.data,256,fp);
            send_pkt.size=strlen(send_pkt.data);
            send_pkt.channel=1;
            send_pkt.lastpkt=0;
            send_pkt.seq_no=seq;
            if(feof(fp))
                send_pkt.lastpkt=1;
            write(sockfd1,&send_pkt,sizeof(send_pkt));
            seq+=send_pkt.size;
            state=0;
            break;
        }
        case 2:
        {
            if(feof(fp))
            {
                flag=1;
                break;
            }
            fgets(send_pkt.data,256,fp);
            send_pkt.size=strlen(send_pkt.data);
            send_pkt.channel=1;
            send_pkt.lastpkt=0;
            send_pkt.seq_no=seq;
            if(feof(fp))
                send_pkt.lastpkt=1;
            write(sockfd2,&send_pkt,sizeof(send_pkt));
            seq+=send_pkt.size;
            state=0;
            break;
        }
        
        }
        if (flag == 1)
            break;
    }
    fclose(fp);
    close(sockfd1);
    close(sockfd2);
    return 0;
}