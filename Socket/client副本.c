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

#define SP_PORT     8890
#define BUFFER_SIZE 4096
#define SERVER_ADDR "127.0.0.1"
char* ROOTKIT = "/Volumes/F/test/123.txt\n"     // '\n' just for read easier(line by line).

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
    
//    readbuf(sock_cli, recvbuf, BUFFER_SIZE);
//    printf("value is [%s]", recvbuf);
//    memset(recvbuf, 0, sizeof(recvbuf));

//    while (1)
//    {
//        printf("Yes2\n");
//        int recvd = recv(sock_cli, sock_cli, sizeof(recvbuf),0);
//        printf("%s\n", recvbuf);
//        if(strcmp(recvbuf,"exit\n")==0 || strcmp(recvbuf,"exec exit\n")==0)
//            break;
//        fputs(recvbuf, stdout);
//        
//        send(sock_cli, sendbuf, strlen(sendbuf),0);     //Tell the remote machine it is ready again
//        memset(recvbuf, 0, sizeof(recvbuf));
//    }
    
//    while(1)
//    {
//        int cnt = (int)recv(sock_cli, recvbuf, sizeof(recvbuf), 0);
////        cnt = (int)recv(sock_cli, recvbuf,sizeof(recvbuf), 0);
//        if( cnt >0 )
//        {
//            printf("Yes3\n");
//            if(cnt == 1){
//                printf("receve %d %d %d %d %d\n", recvbuf[0], recvbuf[1], recvbuf[2], recvbuf[3], recvbuf[4]);
//            }
//            printf("%d[%s]\n", cnt, recvbuf);
//            if(strcmp(recvbuf,"e\n")==0 || strcmp(recvbuf,"exec exit\n")==0)
//                break;
////            break;
////            send(sock_cli, sendbuf, strlen(sendbuf),0);     //Tell the remote machine it is ready again
//            memset(recvbuf, 0, sizeof(recvbuf));
//            
//        }
//        else
//       {
//           continue;
//        }
//    }
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