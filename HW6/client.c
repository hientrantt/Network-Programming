#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<ctype.h>

//Create a Socket for server communication
short SocketCreate(void);
//try to connect with server
int SocketConnect(int, char*, char*);
// Send the data to the server and set the timeout of 20 seconds
int SocketSend(int, char*, short);
//receive the data from the server
int SocketReceive(int, char*, short);

int main(int argc, char *argv[])
{
    if(argc!=3){
        printf("Invalid arguments\n");
        return 1;
    }

    int hSocket, read_size;
    struct sockaddr_in server;
    char SendToServer[100] = {0};
    char server_reply[200] = {0};
    //Create socket
    hSocket = SocketCreate();
    if(hSocket == -1)
    {
        printf("Could not create socket\n");
        return 1;
    }
    printf("Socket is created\n");
    //Connect to remote server
    if (SocketConnect(hSocket, argv[1], argv[2]) < 0)
    {
        perror("connect failed.\n");
        return 1;
    }
    printf("Sucessfully conected with server\n");

    printf("Username: ");
    while (1){
        memset(SendToServer, '\0', strlen(SendToServer));
        memset(server_reply, '\0', strlen(server_reply));
        // printf("Enter the Message: ");
        // gets(SendToServer);
        fgets(SendToServer, sizeof(char)*50, stdin);
        //Send data to the server
        SocketSend(hSocket, SendToServer, strlen(SendToServer));
        //Received the data from the server
        read_size = SocketReceive(hSocket, server_reply, 200);
        printf("%s",server_reply);
        if(strcmp(server_reply, "byeee")==0){
            printf("\n");
            break;
        }
    }
    close(hSocket);
    shutdown(hSocket,0);
    shutdown(hSocket,1);
    shutdown(hSocket,2);
    return 0;
}

short SocketCreate(void)
{
    short hSocket;
    printf("Create the socket\n");
    hSocket = socket(AF_INET, SOCK_STREAM, 0);
    return hSocket;
}

int SocketConnect(int hSocket, char* ipAddress , char* portNumbeStr)
{
    int iRetval=-1;
    // int ServerPort = 90190;
    int portNumber = atoi(portNumbeStr);
    int ServerPort = portNumber;
    struct sockaddr_in remote= {0};
    // remote.sin_addr.s_addr = inet_addr("127.0.0.1"); //Local Host
    remote.sin_addr.s_addr = inet_addr("127.0.0.1"); //Local Host
    remote.sin_family = AF_INET;
    remote.sin_port = htons(ServerPort);
    iRetval = connect(hSocket,(struct sockaddr *)&remote,sizeof(struct sockaddr_in));
    return iRetval;
}

int SocketSend(int hSocket,char* Rqst,short lenRqst)
{
    int shortRetval = -1;
    struct timeval tv;
    tv.tv_sec = 20;  /* 20 Secs Timeout */
    tv.tv_usec = 0;
    if(setsockopt(hSocket,SOL_SOCKET,SO_SNDTIMEO,(char *)&tv,sizeof(tv)) < 0)
    {
        printf("Time Out\n");
        return -1;
    }
    shortRetval = send(hSocket, Rqst, lenRqst, 0);
    return shortRetval;
}

int SocketReceive(int hSocket,char* Rsp,short RvcSize)
{
    int shortRetval = -1;
    struct timeval tv;
    tv.tv_sec = 20;  /* 20 Secs Timeout */
    tv.tv_usec = 0;
    if(setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO,(char *)&tv,sizeof(tv)) < 0)
    {
        printf("Time Out\n");
        return -1;
    }
    shortRetval = recv(hSocket, Rsp, RvcSize, 0);
    // printf("%s",Rsp);
    return shortRetval;
}
