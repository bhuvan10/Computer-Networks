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
#define PORT 8888  // The port on which to send data

typedef struct packet2
{
    int sq_no;
    int lastpkt;
    int sz;
    int sender;
    int type;
    char data[BUFLEN];
} pkt;
void die(char *s)
{
    perror(s);
    exit(1);
}
int main(void)
{
    int sockfd = 0;
    int bytesReceived = 0;
    char recvBuff[256];
    struct timeval timeout;
    fd_set fds;
    unsigned char buff_offset[10]; // buffer to send the File offset value
    unsigned char buff_command[2]; // buffer to send the Complete File (0) or Partial File Command(1).int offset; // required to get the user input for offset in case of partial file command
    int command;                   // required to get the user input for command
    memset(recvBuff, '0', sizeof(recvBuff));
    struct sockaddr_in serv_addr;
    /* Create a socket first */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }
    /* Initialize sockaddr_in data structure */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT); // port
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    /* Attempt a connection */
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }
    /* Create file where data will be stored */
    FILE *fp;
    fp = fopen("id.txt", "r");
    if (NULL == fp)
    {
        printf("Error opening file");
        return 1;
    }
    int state = 0;
    int drop_flag = 0;
    FILE *previous = NULL;
    int nread = 0;
    pkt send_pkt, rcv_ack;
    int ready = 0;
    while (1)
    {
        switch (state)
        {
        case 0:
            // wait for sending packet with seq.no. 0
            previous = fp;
            if (drop_flag == 0)
            {
                fgets(send_pkt.data,BUFLEN,fp);
                send_pkt.sz=strlen(send_pkt.data);
            }
            send_pkt.lastpkt = 0;
            send_pkt.type = 0;
            // fgets(send_pkt.data, sizeof(send_pkt), stdin);
            send_pkt.data[send_pkt.sz] = '\0';
            if (feof(fp))
            {
                send_pkt.lastpkt = 1;
            }
            printf("message 0: bytes sent %d\n", send_pkt.sz);
            send_pkt.sq_no = 0;
            if (write(sockfd, &send_pkt, sizeof(send_pkt)) == 0)
            {
                die("write()");
            }
            state = 1;

            break;
        case 1: // waiting for ACK 0
            FD_ZERO(&fds);
            FD_SET(sockfd, &fds);

            timeout.tv_sec = 5; // 5-second timeout
            timeout.tv_usec = 0;
            ready = select(sockfd + 1, &fds, NULL, NULL, &timeout);
            printf("%d\n", ready);
            if (ready == -1)
            {
                die("select");
            }
            else if (ready == 0)
            {
                printf("Timeout occurred. Resending packet 0...\n");
                fp = previous;
                drop_flag = 1;
                state = 0; // Go back to state 0 for retransmission
                break;
            }

            if (read(sockfd, &rcv_ack, sizeof(rcv_ack)) == 0)
            {
                die("read()");
            }
            if (rcv_ack.sq_no == 0)
            {
                printf("Received ack seq. no. %d\n", rcv_ack.sq_no);
                if (feof(fp))
                {
                    close(sockfd);
                    return 0;
                }
                drop_flag = 0;
                state = 2;
                break;
            }
            break;
        case 2:
            previous = fp;
            if (drop_flag == 0)
            {
                fgets(send_pkt.data,BUFLEN,fp);
                send_pkt.sz=strlen(send_pkt.data);
            }
            send_pkt.lastpkt = 0;
            send_pkt.type = 0;
            // fgets(send_pkt.data, sizeof(send_pkt), stdin);
            if (feof(fp))
            {
                send_pkt.lastpkt = 1;
            }
            send_pkt.data[send_pkt.sz] = '\0';
            printf("message 1: %d\n", send_pkt.sz);
            // wait for sending packet with seq. no. 1
            send_pkt.sq_no = 1;
            if (write(sockfd, &send_pkt, sizeof(send_pkt)) == 0)
            {
                die("write()");
            }
            state = 3;
            break;
        case 3: // waiting for ACK 1
            FD_ZERO(&fds);
            FD_SET(sockfd, &fds);

            timeout.tv_sec = 5; // 5-second timeout
            timeout.tv_usec = 0;
            ready = select(sockfd + 1, &fds, NULL, NULL, &timeout);
            printf("%d\n", ready);
            if (ready == -1)
            {
                die("select");
            }
            else if (ready == 0)
            {
                printf("Timeout occurred. Resending packet 1...\n");
                fp = previous;
                drop_flag = 1;
                state = 2; // Go back to state 0 for retransmission
                break;
            }
            if (read(sockfd, &rcv_ack, sizeof(rcv_ack)) == 0)
            {
                die("recvfrom()");
            }
            if (rcv_ack.sq_no == 1)
            {
                printf("Received ack seq. no. %d\n", rcv_ack.sq_no);
                if (feof(fp))
                {
                    close(sockfd);
                    return 0;
                }
                drop_flag = 0;
                state = 0;
                break;
            }
            break;
        }
    }
    fclose(fp);
    return 0;
}