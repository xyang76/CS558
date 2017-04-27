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
#include <pthread.h>

#define SP_PORT  9999
#define QUEUE   20
#define BUFFER_SIZE 1024

int runable;
void thread(int conn)
{
    char buffer[BUFFER_SIZE];
　　 while(1)
    {
        memset(buffer,0,sizeof(buffer));
        int len = recv(conn, buffer, sizeof(buffer),0);
        if(strcmp(buffer,"exit\n")==0) {
            runable = 0;
            break;
        }
        fputs(buffer, stdout);
        send(conn, buffer, len, 0);
    }
    close(conn);
}

int main()
{
    int server_sockfd = socket(AF_INET,SOCK_STREAM, 0);
    
    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(SP_PORT);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);    

    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);
    
    int rv;
    pthread_t tid;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

    if(bind(server_sockfd,(struct sockaddr *)&server_sockaddr,sizeof(server_sockaddr))==-1)
    {
        exit(1);
    }

    if(listen(server_sockfd,QUEUE) == -1)
    {
        exit(1);
    }
    
    runable = 1;
    while(runable){
        printf("ready\n");
        int conn = accept(server_sockfd, (struct sockaddr*)&client_addr, &length);
        if(conn<0)
        {
            exit(1);
        }
        printf("get a new connection from %s\n", inet_ntoa(client_addr.sin_addr));
        rv=pthread_create(&tid,&attr,(void *) thread, (void*)conn);
    }
    pthread_attr_destroy(&attr);
    close(server_sockfd);
    return 0;
}