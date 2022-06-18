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
#include <stdbool.h>
#include <semaphore.h>
#include <time.h>

#define CHATSIZE 8192

enum {SECS_TO_SLEEP = 0, NSEC_TO_SLEEP = 125};


typedef struct message {
    char name[12];
    enum {quit = 1, chat = 2, sticker = 3}command;
    char stickerID;
    char message[1024];
    char time[20];
} message_t;

typedef struct pair {
    int connfd;
    char name[12];
} pair_t;

volatile int TotalPeople = 0; // present total people in chatroom
char people[1024][1024]; // person in chatroom
message_t chatHistory[CHATSIZE]; // chat message history
volatile int chatCnt_global = 0; // total message count

sem_t mutex_chatCnt_global;
sem_t mutex_TotalPeople;

int listenfd = 0;

void *thread_write(void *vargp);
void *thread_read(void *vargp);
void *thread_create_user(void *vargp);

int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr; 
    struct sockaddr *clientaddr = NULL;
    pthread_t tid;

    sem_init(&mutex_chatCnt_global, 0, 1); // mutex_chatCnt_global = 1
    sem_init(&mutex_TotalPeople, 0, 1); // mutex_TotalPeople = 1

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 10); 

    memset(people, 0, 1024 * 1024);
    memset(chatHistory, 0, sizeof(message_t) * 1024);

    long cnt = 0;

    while(1) {
        int *connfd = malloc(sizeof(int));
        *connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
        pthread_create(&tid, NULL, thread_create_user, connfd);
        sleep(1);
    }
}

void *thread_create_user(void *vargp) {
    time_t rawtime;
    struct tm *info;
    char recvBuff[1025], Name[12], message[1024], present_time[20];
    pthread_t tid;

    int connfd = *(int *)vargp;
    pthread_detach(pthread_self());
    free(vargp);

    memset(recvBuff, 0, sizeof(recvBuff));
    read(connfd, recvBuff, sizeof(recvBuff) - 1);

    strcpy(Name, strtok(recvBuff, ":"));
    int idx = -1;
    for (int i = 0; i < TotalPeople; ++i) {
        if (!strcmp(people[i], Name))
            idx = i;
    }

    if (idx == -1) {
        sem_wait(&mutex_TotalPeople);
        idx = TotalPeople;
        TotalPeople++;
        sem_post(&mutex_TotalPeople);

        strcpy(people[idx], Name);

        time(&rawtime);
        info = localtime(&rawtime);
        strftime(present_time, 20, "%H:%M:%S", info);
        memset(message, 0, sizeof(message)); 
        sprintf(message, "%s enter the chatroom", Name);

        sem_wait(&mutex_chatCnt_global);
        chatHistory[chatCnt_global].command = chat;
        chatHistory[chatCnt_global].stickerID = -1;
        strcpy(chatHistory[chatCnt_global].name, "server");
        strcpy(chatHistory[chatCnt_global].message, message);
        strcpy(chatHistory[chatCnt_global].time, present_time);
        chatCnt_global = (chatCnt_global + 1) & (CHATSIZE - 1);
        sem_post(&mutex_chatCnt_global);
    }

    pair_t *pair = malloc (sizeof(pair_t));
    pair->connfd = connfd;
    strcpy(pair->name, Name);
    if (!strcmp(recvBuff + strlen(Name) + 1, "read")) {
        // create read thread
        pthread_create(&tid, NULL, thread_read, pair);
    } else {
        // write read thread
        pthread_create(&tid, NULL, thread_write, pair);
    }

    return NULL;
}

void *thread_read(void *vargp) {
    time_t rawtime;
    struct tm *info;
    char recvBuff[10240], Name[12], message[1024], present_time[1024];
    
    strcpy(Name, ((pair_t *)vargp)->name);
    int connfd = ((pair_t *)vargp)->connfd;
    pthread_detach(pthread_self());

    while (1) {
        memset(recvBuff, 0, sizeof(recvBuff));
        read(connfd, recvBuff, sizeof(recvBuff) - 1);
        
        // leave the chatroom
        if (!strlen(recvBuff))
            break;
    
        time(&rawtime);
        info = localtime(&rawtime);
        strftime(present_time, 20, "%H:%M:%S", info);

        sem_wait(&mutex_chatCnt_global);
        if (recvBuff[0] == sticker) {
            chatHistory[chatCnt_global].command = sticker;
            chatHistory[chatCnt_global].stickerID = recvBuff[1];
            strcpy(chatHistory[chatCnt_global].message, "");
            strcpy(chatHistory[chatCnt_global].name, Name);
            strcpy(chatHistory[chatCnt_global].time, present_time);
            chatCnt_global = (chatCnt_global + 1) & (CHATSIZE - 1);
        } else {
            chatHistory[chatCnt_global].command = recvBuff[0];
            chatHistory[chatCnt_global].stickerID = -1;
            strcpy(chatHistory[chatCnt_global].message, recvBuff + 1);
            strcpy(chatHistory[chatCnt_global].name, Name);
            strcpy(chatHistory[chatCnt_global].time, present_time);
            chatCnt_global = (chatCnt_global + 1) & (CHATSIZE - 1);
        }
        sem_post(&mutex_chatCnt_global);
    }
    
    time(&rawtime);
    info = localtime(&rawtime);
    strftime(present_time, 20, "%H:%M:%S", info);

    memset(recvBuff, 0, sizeof(recvBuff));
    sprintf(recvBuff, "%s leave the chatroom", Name);

    sem_wait(&mutex_chatCnt_global);
    chatHistory[chatCnt_global].command = quit;
    chatHistory[chatCnt_global].stickerID = -1;
    strcpy(chatHistory[chatCnt_global].name, "server");
    strcpy(chatHistory[chatCnt_global].message, recvBuff);
    strcpy(chatHistory[chatCnt_global].time, present_time);
    chatCnt_global = (chatCnt_global + 1) & (CHATSIZE - 1);
    sem_post(&mutex_chatCnt_global);

    free(vargp);
    close(connfd);

    return NULL;
}

void *thread_write(void *vargp) {
    char sendBuff[10240], checkOnlineBuff[2];

    char Name[12];
    strcpy(Name, ((pair_t *)vargp)->name);
    int connfd = ((pair_t *)vargp)->connfd;
    int chatCnt = chatCnt_global;

    struct timespec remaining, request = {SECS_TO_SLEEP, NSEC_TO_SLEEP};

    pthread_detach(pthread_self());
    
    while (1) {
        if (chatCnt_global != chatCnt) {
            memset(sendBuff, 0, sizeof(sendBuff)); 
            memset(checkOnlineBuff, 0, 2); 
            if (chatHistory[chatCnt].command == sticker) {
                // buff[] = command name:stickerID(time)
                sprintf(sendBuff, "%c%s:%c(%s)", chatHistory[chatCnt].command, chatHistory[chatCnt].name, chatHistory[chatCnt].stickerID, chatHistory[chatCnt].time);
                write(connfd, sendBuff, strlen(sendBuff) + 1);
                read(connfd, checkOnlineBuff, 2);
            } else {
                // buff[] = command name:message(time)
                sprintf(sendBuff, "%c%s:%s(%s)", chatHistory[chatCnt].command, chatHistory[chatCnt].name, chatHistory[chatCnt].message, chatHistory[chatCnt].time);
                write(connfd, sendBuff, strlen(sendBuff) + 1);
                read(connfd, checkOnlineBuff, 2);
                if (strlen(checkOnlineBuff) == 0) {
                    break;
                }
            }
            chatCnt = (chatCnt + 1) & (CHATSIZE - 1);
        }
        
        nanosleep(&request, &remaining);
    }


    free(vargp);
    close(connfd);

    return NULL;
}
