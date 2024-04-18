/* Server program for broken ftp */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
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
int should_drop_packet()
{
    // Simulate packet loss with a 20% probability
    return (rand() % 100) < 50;
}
int main(void)
{
    int listenfd = 0;
    int connfd = 0;
    struct sockaddr_in serv_addr;
    pkt rcv_pkt;pkt1 send_ack;
    int numrv;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    printf("Socket retrieve success\n");
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(rcv_pkt.data, '0', sizeof(rcv_pkt.data));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5001);
    bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (listen(listenfd, 10) == -1)
    {
        printf("Failed to listen\n");
        return -1;
    }
    int state=0;
     connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
        FILE *fp = fopen("output.c", "w");
        if (fp == NULL)
        {
            printf("File opern error");
            return 1;
        }
        /* Read data from file and send it */
        int bytesReceived = 0;
    while (1)
    {
        switch(state)
        {
            case 0:
            {
                    if(!should_drop_packet())
                    {
                    bytesReceived = read(connfd, &rcv_pkt, sizeof(rcv_pkt));
                    printf("Bytes received %d with seq no. %d\n", rcv_pkt.size,rcv_pkt.seq_no);
                    fwrite(rcv_pkt.data, 1, rcv_pkt.size, fp);
                    send_ack.seq_no=0;
                    //send_ack.type=1;
                    write(connfd,&send_ack,sizeof(send_ack));
                    state=1;
                    }
                    else
                    {
                        printf("Package dropped\n");
                    }
                    break;
            }
            case 1:
            {
                    if(!should_drop_packet())
                    {
                    bytesReceived = read(connfd, &rcv_pkt, sizeof(rcv_pkt));
                    printf("Bytes received %d with seq no. %d\n", rcv_pkt.size,rcv_pkt.seq_no);
                    fwrite(rcv_pkt.data, 1, rcv_pkt.size, fp);
                    send_ack.seq_no=1;
                    //send_ack.type=1;
                    write(connfd,&send_ack,sizeof(send_ack));
                    state=0;
                    }
                    else
                    {
                        printf("Package dropped\n");
                    }
                    break;
            }
        }
        if(rcv_pkt.lastpkt==1)
            break;       
    }
    close(connfd);
    fclose(fp);
    return 0;
}