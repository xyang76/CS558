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


#define BUFFER_SIZE 4096
#define SP_PORT     8890
#define SERVER_ADDR "127.0.0.1"
char* ROOTKIT = "/Volumes/F/test/hidefile.ko\n";     // '\n' just for read easier(line by line).

static int readbuf(int conn, char* buf, int size);
static int getIntFromBuf(char* buf, int offset);

int main()
{
    int sock_cli = socket(AF_INET,SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    char recvbuf[BUFFER_SIZE];
    int filesize, chunk;
    FILE *fp;    

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SP_PORT); 
    servaddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);  
    
    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
         printf("No0\n");
        exit(1);
    }
    char *req = "obtain\n";                         // Req for obtain a rootkit
    send(sock_cli, req, strlen(req),0); 
    send(sock_cli, ROOTKIT, strlen(ROOTKIT),0); 
    
    memset(recvbuf, 0, sizeof(recvbuf));    
    readbuf(sock_cli, recvbuf, 4);                  // Read file size
    filesize = getIntFromBuf(recvbuf, 0);
     printf("size = %d\n", filesize);
    memset(recvbuf, 0, sizeof(recvbuf));
    fp=fopen("test.txt","w+");                      // Obtain file with buffer
    while(filesize > 0){
        if(filesize > BUFFER_SIZE){
            readbuf(sock_cli, recvbuf, BUFFER_SIZE);
            fwrite(recvbuf, BUFFER_SIZE, 1, fp);
            filesize -= BUFFER_SIZE;
        } else {
            readbuf(sock_cli, recvbuf, filesize);
            fwrite(recvbuf, filesize, 1, fp);
            break;
        }
    }
    
    fclose(fp);
    close(sock_cli);
    return 0;
}


static int readbuf(int conn, char* buf, int size){
    int cur = 0;
    while(cur < size){
        int cnt = recv(conn, buf + cur, size - cur, 0);
        if(cnt > 0){
            cur += cnt;
        } 
    }
    return cur;
}

static int getIntFromBuf(char* buf, int offset){
    int value;    
    value = (int) ( ((buf[offset] & 0xFF)<<24)  
            |((buf[offset+1] & 0xFF)<<16)  
            |((buf[offset+2] & 0xFF)<<8)  
            |(buf[offset+3] & 0xFF));  
    return value;
}