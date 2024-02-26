#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAXPENDING 5
#define BUFFERSIZE 32
char *handle_put(char *request) {
    char key[100], value[100];
    sscanf(request, "put %s %s", key, value);
    FILE *file = fopen("database.txt", "a");
    fprintf(file, "%s %s\n", key, value);
    fclose(file);
    return "OK";
}

// Function to handle get request
char *handle_get(char *request) {
    char key[100], value[100];
    sscanf(request, "get %s", key);
    FILE *file = fopen("database.txt", "r");
    while (fscanf(file, "%s %s", value, key) != EOF) {
        if (strcmp(key, value) == 0) {
            fclose(file);
            return value;
        }
    }
    fclose(file);
    return "Key not found";
}
int main ()
{
/*CREATE A TCP SOCKET*/
int serverSocket = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
if (serverSocket < 0) { printf ("Error while server socketcreation"); 
exit (0); }
printf ("Server Socket Created\n");
/*CONSTRUCT LOCAL ADDRESS STRUCTURE*/
struct sockaddr_in serverAddress, clientAddress;
memset (&serverAddress, 0, sizeof(serverAddress));
serverAddress.sin_family = AF_INET;
serverAddress.sin_port = htons(12345);
serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
printf ("Server address assigned\n");
int temp = bind(serverSocket, (struct sockaddr*) &serverAddress,
sizeof(serverAddress));
if (temp < 0)
{ printf ("Error while binding\n");
exit (0);
}
printf ("Binding successful\n");
int temp1 = listen(serverSocket, MAXPENDING);
if (temp1 < 0)
{ printf ("Error in listen");
exit (0);
}
printf ("Now Listening\n");
char msg[BUFFERSIZE];
int clientLength = sizeof(clientAddress);
while(1){
        int flag=0;

int newSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLength);
if(clientLength < 0){
    printf("Error in client socket");
exit(0);
}
printf("Connection accepted from %s:%d\n",inet_ntoa(clientAddress.sin_addr));
int childpid=0;
if((childpid = fork()) == 0){
close(serverSocket);
while(1){
while((temp=recv(newSocket, msg, BUFFERSIZE, 0))<0);
if(strcmp(msg, ":exit") == 0){
printf("Disconnected from %s:%d\n",inet_ntoa(clientAddress.sin_addr),ntohs(clientAddress.sin_port));
flag=1;
break;}
else {
printf("Client: %s\n", msg);
strcpy(msg,handle_put(msg));
send(newSocket, msg, strlen(msg), 0);
bzero(msg, sizeof(msg));
}
if(flag==1)
break;
}
close(newSocket);
if(flag==1)
break;
}
}
// int clientSocket = accept (serverSocket, (struct sockaddr*)
// &clientAddress, &clientLength);
// if (clientLength < 0) {printf ("Error in client socket"); exit(0);}
// printf ("Handling Client %s\n", inet_ntoa(clientAddress.sin_addr));
// int temp2 = recv(clientSocket, msg, BUFFERSIZE, 0);
// if (temp2 < 0)
// { printf ("problem in temp 2");
// exit (0);
// }
// printf ("%s\n", msg);
// printf ("ENTER MESSAGE FOR CLIENT\n");
// gets (msg);
// int bytesSent = send (clientSocket,msg,strlen(msg),0);
// if (bytesSent != strlen(msg))
// { printf ("Error while sending message to client");
// exit(0);
// }
// close (serverSocket);
// close (clientSocket);
}