#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <string.h>

int TotalPeople = 0;
char chatHistory[1025][1024];
int chatCnt = 0;

void *thread(void *vargp);

int main(int argc, char *argv[])
{
    int listenfd = 0;
    struct sockaddr_in serv_addr; 
    struct sockaddr *clientaddr = NULL;
    pthread_t tid;
    

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 10); 

    while(1)
    {
        int *connfd = malloc(sizeof(int));
        *connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
        pthread_create(&tid, NULL, thread, connfd);
       
        sleep(1);
     }
}

void *thread(void *vargp) {
    char sendBuff[1025], recvBuff[1025];

    int connfd = *(int *)vargp;
    pthread_detach(pthread_self());
    free(vargp);

    memset(sendBuff, 0, sizeof(sendBuff)); 
    strncpy(sendBuff, "Hello World!", 12);
    write(connfd, sendBuff, strlen(sendBuff));
    printf("wwww chatCnt = %d\n", chatCnt);

    memset(recvBuff, 0, sizeof(recvBuff)); 
    read(connfd, recvBuff, sizeof(recvBuff) - 1);
    if (strlen(recvBuff) > 0) {
        strcpy(chatHistory[chatCnt], recvBuff);
        chatCnt++;
    }
            
    close(connfd);

    return NULL;
}
