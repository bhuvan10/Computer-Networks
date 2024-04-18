/* Simple udp server with stop and wait functionality */
#include <stdio.h>  //printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFLEN 256 // Max length of buffer
#define PORT 8882  // The port on which to listen for incoming data
void die(char *s)
{
    perror(s);
    exit(1);
}
typedef struct packet1
{
    int sq_no;
} ACK_PKT;
typedef struct packet2
{
    int sq_no;
    int lastpkt;
    int sz;
    int sender;
    char data[BUFLEN];   
} DATA_PKT;
int should_drop_packet()
{
    // Simulate packet loss with a 20% probability
    return (rand() % 100) < 20;
}
int count=0;
int main(void)
{
    struct sockaddr_in si_me, si_other;
    int s, i, slen = sizeof(si_other), recv_len;
    // char buf[BUFLEN];
    DATA_PKT rcv_pkt;
    ACK_PKT ack_pkt;
    // create a UDP socket
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
    // zero out the structure
    memset((char *)&si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    // bind socket to port
    if (bind(s, (struct sockaddr *)&si_me, sizeof(si_me)) == -1)
    {
        die("bind");
    }
    int state = 0;
    FILE* fp=fopen("list.txt","w");
    while (1)
    {
        if(count==2)
        {
            fclose(fp);
            break;
        }
        switch (state)
        {
        case 0:
        {
            printf("Waiting for packet 0 from sender... 1\n");
            fflush(stdout);
            // try to receive some data, this is a blocking call
            if ((recv_len = recvfrom(s, &rcv_pkt, BUFLEN, 0, (struct sockaddr *)&si_other, &slen)) == -1)
            {
                die("recvfrom()");
            }
            if (rcv_pkt.sq_no == 0&&rcv_pkt.sender==1)
            {
                printf("Packet received with seq. no. %d and Packet bytes is = %d\n", rcv_pkt.sq_no, rcv_pkt.sz);
                
                if (!should_drop_packet())
                {
                    fprintf(fp,rcv_pkt.data,rcv_pkt.sz);
                    ack_pkt.sq_no = 0;
                    if (sendto(s, &ack_pkt, recv_len, 0, (struct sockaddr *)&si_other,
                               slen) == -1)
                    {
                        die("sendto()");
                    }
                    //printf("which packet :%d\n",rcv_pkt.lastpkt);
                     if(rcv_pkt.lastpkt==1)
                    {
                        count++;
                    }
                    state = 1;
                }
                else
                {
                    printf("Packet Droped with sequence 0\n");
                    state = 0;
                }
                break;
            }
            state=0;
            break;
        }
        case 1:
        {
            printf("Waiting for packet 0 from sender...2\n");
            fflush(stdout);
            // try to receive some data, this is a blocking call
            if ((recv_len = recvfrom(s, &rcv_pkt, BUFLEN, 0, (struct sockaddr *)&si_other, &slen)) == -1)
            {
                die("recvfrom()");
            }
            if (rcv_pkt.sq_no == 0&&rcv_pkt.sender==2)
            {
                printf("Packet received with seq. no. %d and Packet bytes is = %d\n", rcv_pkt.sq_no, rcv_pkt.sz);

               
                if (!should_drop_packet())
                {
                     fprintf(fp,rcv_pkt.data,rcv_pkt.sz);
                     ack_pkt.sq_no = 1;
                    if (sendto(s, &ack_pkt, recv_len, 0, (struct sockaddr *)&si_other,
                               slen) == -1)
                    {
                        die("sendto()");
                    }
                    //printf("which packet :%d\n",rcv_pkt.lastpkt);
                     if(rcv_pkt.lastpkt==1)
                    {
                        count++;
                    }
                    state = 2;
                }
                else
                {
                    printf("Packet Droped with sequence 1\n");
                    state = 1;
                }
                break;
            }
            state=1;
            break;
        }
        case 2:
        {
            printf("Waiting for packet 1 from sender... 1\n");
            fflush(stdout);
            // try to receive some data, this is a blocking call
            if ((recv_len = recvfrom(s, &rcv_pkt, BUFLEN, 0, (struct sockaddr *)&si_other, &slen)) == -1)
            {
                die("recvfrom()");
            }
            if (rcv_pkt.sq_no == 1&&rcv_pkt.sender==0)
            {
                printf("Packet received with seq. no. %d and Packet bytes is = %d\n", rcv_pkt.sq_no, rcv_pkt.sz);
                
                if (!should_drop_packet())
                {
                    fprintf(fp,rcv_pkt.data,rcv_pkt.sz);
                    ack_pkt.sq_no = 0;
                    if (sendto(s, &ack_pkt, recv_len, 0, (struct sockaddr *)&si_other,
                               slen) == -1)
                    {
                        die("sendto()");
                    }
                    //printf("which packet :%d\n",rcv_pkt.lastpkt);
                     if(rcv_pkt.lastpkt==1)
                    {
                        count++;
                    }
                    state = 3;
                }
                else
                {
                    printf("Packet Droped with sequence 0\n");
                    state = 2;
                }
                break;
            }
            state=2;
            break;
        }
        case 3:
        {
            printf("Waiting for packet 1 from sender...2\n");
            fflush(stdout);
            // try to receive some data, this is a blocking call
            if ((recv_len = recvfrom(s, &rcv_pkt, BUFLEN, 0, (struct sockaddr *)&si_other, &slen)) == -1)
            {
                die("recvfrom()");
            }
            if (rcv_pkt.sq_no == 1&&rcv_pkt.sender==2)
            {
                printf("Packet received with seq. no. %d and Packet bytes is = %d\n", rcv_pkt.sq_no, rcv_pkt.sz);

               
                if (!should_drop_packet())
                {
                     fprintf(fp,rcv_pkt.data,rcv_pkt.sz);
                     ack_pkt.sq_no = 1;
                    if (sendto(s, &ack_pkt, recv_len, 0, (struct sockaddr *)&si_other,
                               slen) == -1)
                    {
                        die("sendto()");
                    }
                    //printf("which packet :%d\n",rcv_pkt.lastpkt);
                     if(rcv_pkt.lastpkt==1)
                    {
                        count++;
                    }
                    state = 0;
                }
                else
                {
                    printf("Packet Droped with sequence 1\n");
                    state = 3;
                }
                break;
            }
            state=3;
            break;
        }
        }
    }
    close(s);
    return 0;
}