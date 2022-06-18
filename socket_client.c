#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <pthread.h>
#include <semaphore.h>

void *thread_read_chat();
void *thread_send_text();

enum {SECS_TO_SLEEP = 0, NSEC_TO_SLEEP = 125};

bool toQuit = false;

int main(int argc, char *argv[])
{
    int sockfd = 0;
    char recvBuff[1024], message[1024];
    struct sockaddr_in serv_addr;

    if(argc != 3) {
        printf("\n Usage: %s <ip of server> \n",argv[0]);
        return 1;
    }
    
    memset(recvBuff, 0 ,sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 
    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000); 

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0) {   
        printf("\n inet_pton error occured\n");
        return 1;
    }

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\n Error : Connect Failed \n");
        return 1;
    }

    memset(message, 0, 1024);
    sprintf(message, "%s:read", argv[2]);
    write(sockfd, message, strlen(message));

    pthread_t tid_send_text;
    if (pthread_create(&tid_send_text, NULL, thread_send_text, &sockfd) != 0) {
        printf("Error : pthread_create\n");
    }

    sleep(1);

    pthread_t tid_read_chat;
    if (pthread_create(&tid_read_chat, NULL, thread_read_chat, argv) != 0) {
        printf("Error : pthread_create\n");
    }

    pthread_exit(NULL);
    return 0;
}

void *thread_read_chat(void *vargp)
{
    int sockfd = 0;
    char recvBuff[1024], message[1024];
    char **argv = vargp;
    struct sockaddr_in serv_addr;

    pthread_detach(pthread_self());
    memset(recvBuff, 0 ,sizeof(recvBuff));

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Error : Could not create socket \n");
        return NULL;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000);

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0) {   
        printf("\n inet_pton error occured\n");
        return NULL;
    }

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\n Error : Connect Failed \n");
        return NULL;
    }

    memset(message, 0, 1024);
    sprintf(message, "%s:write", argv[2]);
    write(sockfd, message, strlen(argv[2]));

    while (!toQuit) {
        memset(recvBuff, 0, sizeof(recvBuff));
        read(sockfd, recvBuff, sizeof(recvBuff));
        write(sockfd, "1", 2);
        printf("%s\n", recvBuff);
    }
    write(sockfd, "", 2);
    close(sockfd);
    return NULL;
}

void *thread_send_text(void *vargp)
{
    int sockfd = *(int *)vargp;
    char message[1024];

    struct timespec remaining, request = {SECS_TO_SLEEP, NSEC_TO_SLEEP};

    pthread_detach(pthread_self());
    while (!toQuit) {
        memset(message, 0, sizeof(message)); 
        fgets(message, 1024, stdin);
        message[strlen(message) - 1] = 0;
        if (message[0] == '/') {
            if (!strcmp(message, "/quit")) {
                toQuit = true;
                message[0] = 0;
            }
        }
        write(sockfd, message, strlen(message) + 1);
        
        nanosleep(&request, &remaining);
    }
    close(sockfd);
    return NULL;
}
