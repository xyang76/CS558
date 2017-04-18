#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>

#define SP_PORT  6649
#define BUFFER_SIZE 1024

int main()
{
    int sock_cli = socket(AF_INET,SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    char sendbuf[BUFFER_SIZE];
    char recvbuf[BUFFER_SIZE];
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SP_PORT); 
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");  
    printf("Yes0\n");
    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        exit(1);
    }
    char * sendv = "Hello!\n";
    while(1){
        send(sock_cli, sendv, strlen(sendv),0); 
        int len = recv(sock_cli, recvbuf, sizeof(recvbuf),0);
        printf("Receive %s\n", recvbuf);
        if(len > 0){
            printf("Yes %d\n", len);
            fputs(recvbuf, stdout);
            if(strncmp(recvbuf,"exit", 4)==0)
                break;
            fputs(recvbuf, stdout);
            memset(recvbuf, 0, sizeof(recvbuf));
        }
    }

//    while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
//    {
//        
//        if(strcmp(sendbuf,"exit\n")==0)
//            break;
//        send(sock_cli, sendbuf, strlen(sendbuf),0); 
//        fputs(recvbuf, stdout);
//
//        memset(sendbuf, 0, sizeof(sendbuf));
//        memset(recvbuf, 0, sizeof(recvbuf));
//    }

    close(sock_cli);
    return 0;
}