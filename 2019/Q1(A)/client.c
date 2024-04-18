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
    int type;
} pkt;
typedef struct packet1
{
    int seq_no;
} pkt1;
int main(void)
{
    int sockfd = 0;
    pkt send_pkt;
    pkt1 rcv_ack;
    // unsigned char buff_offset[10]; // buffer to send the File offset value
    // unsigned char buff_command[2]; // buffer to send the Complete File (0) or Partial File Command (1).
    // int offset;                    // required to get the user input for offset in case of partial file command
    // int command;                   // required to get the user input for command
    memset(send_pkt.data, '0', sizeof(send_pkt.data));
    struct sockaddr_in serv_addr;
    struct timeval timeout;
    fd_set fds;
    /* Create a socket first */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }
    /* Initialize sockaddr_in data structure */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5001); // port
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    /* Attempt a connection */
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
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
            if (feof(fp))
            {
                flag = 1;
                break;
            }
            send_pkt.lastpkt = 0;
            send_pkt.seq_no = 0;
            if (drop_flag != 1)
                nread = fread(send_pkt.data, 1, 256, fp);
            send_pkt.size = nread;
            if (feof(fp))
                send_pkt.lastpkt = 1;
            printf("Bytes read %d  and was it dropped %d\n", nread, drop_flag);
            /* If read was success, send data. */
            if (nread > 0)
            {
                printf("Sending \n");
                write(sockfd, &send_pkt, sizeof(send_pkt));
            }
            /*
             * There is something tricky going on with read ..
             * Either there was error, or we reached end of file.
             */
            state = 1;
            break;
        }
        case 1:
        {
            FD_ZERO(&fds);
            FD_SET(sockfd, &fds);

            timeout.tv_sec = 1; // Set timeout to 1 second
            timeout.tv_usec = 0;
            ready = select(sockfd + 1, &fds, NULL, NULL, &timeout);

            if (ready == -1)
            {
                die("select");
            }
            else if (ready == 0)
            {
                printf("Timeout occurred. Resending packet 0...\n");
                drop_flag = 1;
                state = 0; // Go back to state 0 for retransmission
                break;
            }
            bytesReceived = read(sockfd, &rcv_ack, sizeof(rcv_ack));
            if (bytesReceived < 0)
            {
                die("read");
            }
            if (bytesReceived == 0)
            {
                printf("Connection closed by server.\n");
                exit(1);
            }
            if (rcv_ack.seq_no == 0)
            {
                printf("Acknowledgement received of seq no. : %d\n", rcv_ack.seq_no);
                drop_flag = 0;
                state = 0;
            }
            else
            {
                state = 1;
            }
            break;
        }
        case 2:
        {
            if (feof(fp))
            {
                flag = 1;
                break;
            }
            send_pkt.lastpkt = 0;
            send_pkt.seq_no = 1;
            if (drop_flag != 1)
                nread = fread(send_pkt.data, 1, 256, fp);
            send_pkt.size = nread;
            if (feof(fp))
                send_pkt.lastpkt = 1;
            printf("Bytes read %d and was it dropped %d\n", nread, drop_flag);
            /* If read was success, send data. */
            if (nread > 0)
            {
                printf("Sending \n");
                write(sockfd, &send_pkt, sizeof(send_pkt));
            }
            /*
             * There is something tricky going on with read ..
             * Either there was error, or we reached end of file.
             */
            state = 3;
            break;
        }
        case 3:
        {
            FD_ZERO(&fds);
            FD_SET(sockfd, &fds);

            timeout.tv_sec = 1; // Set timeout to 1 second
            timeout.tv_usec = 0;
            ready = select(sockfd + 1, &fds, NULL, NULL, &timeout);

            if (ready == -1)
            {
                die("select");
            }
            else if (ready == 0)
            {
                printf("Timeout occurred. Resending packet 0...\n");
                drop_flag = 1;
                state = 0; // Go back to state 0 for retransmission
                break;
            }
            bytesReceived = read(sockfd, &rcv_ack, sizeof(rcv_ack));
            if (bytesReceived < 0)
            {
                die("read");
            }
            if (bytesReceived == 0)
            {
                printf("Connection closed by server.\n");
                exit(1);
            }
            if (rcv_ack.seq_no == 0)
            {
                printf("Acknowledgement received of seq no. : %d\n", rcv_ack.seq_no);
                drop_flag = 0;
                state = 2;
            }
            else
            {
                state = 1;
            }
            break;
        }
        }
        if (flag == 1)
            break;
    }
    fclose(fp);
    close(sockfd);
    return 0;
}