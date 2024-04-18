/*
Simple udp client with stop and wait functionality
*/
#include <stdio.h>  //printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFLEN 256 // Max length of buffer
#define PORT 8882  // The port on which to send data
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
void die(char *s)
{
    perror(s);
    exit(1);
}
int main(void)
{
    struct sockaddr_in si_other;
    int s, i, slen = sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];
    DATA_PKT send_pkt, rcv_ack;
    send_pkt.sender=1;
    struct timeval timeout;
    fd_set fds;
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
    memset((char *)&si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
    si_other.sin_addr.s_addr = inet_addr("127.0.0.1");
    int state = 0;
    int ready = 0;
    FILE *fp = fopen("name.txt", "rb");
    FILE* previous=fp;
    if (fp == NULL)
    {
        printf("File opern error");
        return 1;
    }
    int nread = 0;
    int drop_flag=0;
    while (1)
    {
        switch (state)
        {
        case 0:
            // wait for sending packet with seq.no. 0
            previous=fp;
            if(drop_flag==0)
            nread = fread(send_pkt.data, 1, 256, fp);
            send_pkt.sz=nread;
            send_pkt.lastpkt=0;
            // fgets(send_pkt.data, sizeof(send_pkt), stdin);
            send_pkt.data[nread]='\0';
            if(feof(fp))
            {
                send_pkt.lastpkt=1;
            }
            printf("message 0: bytes sent %d\n", send_pkt.sz);
            send_pkt.sq_no = 0;
            if (sendto(s, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)&si_other, slen) == -1)
            {
                die("sendto()");
            }
            state = 1;

            break;
        case 1: // waiting for ACK 0
            FD_ZERO(&fds);
            FD_SET(s, &fds);

            timeout.tv_sec = 1; // 5-second timeout
            timeout.tv_usec = 0;
            ready = select(s + 1, &fds, NULL, NULL, &timeout);
            printf("%d\n", ready);
            if (ready == -1)
            {
                die("select");
            }
            else if (ready == 0)
            {
                printf("Timeout occurred. Resending packet 0...\n");
                fp=previous;
                drop_flag=1;
                state = 0; // Go back to state 0 for retransmission
                break;
            }

            if (recvfrom(s, &rcv_ack, sizeof(rcv_ack), 0, (struct sockaddr *)&si_other, &slen) == -1)
            {
                die("recvfrom()");
            }
            if (rcv_ack.sq_no == 0)
            {
                printf("Received ack seq. no. %d\n", rcv_ack.sq_no);
                if (feof(fp))
                {
                    close(s);
                    return 0;
                }
                drop_flag=0;
                state = 2;
                break;
            }
            break;
        case 2:
            previous=fp;
            if(drop_flag==0)
            nread = fread(send_pkt.data, 1, 256, fp);
            send_pkt.sz=nread;
            send_pkt.lastpkt=0;
            // fgets(send_pkt.data, sizeof(send_pkt), stdin);
            if(feof(fp))
            {
                send_pkt.lastpkt=1;
            }
            send_pkt.data[nread]='\0';
            printf("message 1: %d\n", send_pkt.sz);
            // wait for sending packet with seq. no. 1
            send_pkt.sq_no = 1;
            if (sendto(s, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)&si_other, slen) == -1)
            {
                die("sendto()");
            }
            state = 3;
            break;
        case 3: // waiting for ACK 1
            FD_ZERO(&fds);
            FD_SET(s, &fds);

            timeout.tv_sec = 5; // 5-second timeout
            timeout.tv_usec = 0;
            ready = select(s + 1, &fds, NULL, NULL, &timeout);
            printf("%d\n", ready);
            if (ready == -1)
            {
                die("select");
            }
            else if (ready == 0)
            {
                printf("Timeout occurred. Resending packet 1...\n");
                fp=previous;
                drop_flag=1;
                state = 2; // Go back to state 0 for retransmission
                break;
            }
            if (recvfrom(s, &rcv_ack, sizeof(rcv_ack), 0, (struct sockaddr *)&si_other, &slen) == -1)
            {
                die("recvfrom()");
            }
            if (rcv_ack.sq_no == 1)
            {
                printf("Received ack seq. no. %d\n", rcv_ack.sq_no);
                if (feof(fp))
                {
                    close(s);
                    return 0;
                }
                drop_flag=0;
                state = 0;
                break;
            }
            break;
        }
    }
    close(s);
    return 0;
}