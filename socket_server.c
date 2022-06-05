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

typedef struct message {
    char name[1024];
    char message[1024];
    bool isLeaveMessage;
    char time[20];
} message_t;

typedef struct person {
    char name[1024];
    int chatCnt;
} person_t;

volatile int TotalPeople = 0; // present total people in chatroom
person_t people[1024]; // person in chatroom
message_t chatHistory[CHATSIZE]; // chat message history
volatile int chatCnt_global = 0; // total message count

sem_t mutex_chatCnt_global;
sem_t mutex_TotalPeople;

int listenfd = 0;

void *thread(void *vargp);

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

    memset(people, 0, sizeof(person_t) * 1024);
    memset(chatHistory, 0, sizeof(message_t) * 1024);

    while(1)
    {
        int *connfd = malloc(sizeof(int));
        *connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
        pthread_create(&tid, NULL, thread, connfd);
       
        sleep(1);
     }
}

void *thread(void *vargp) {
    char sendBuff[102400], recvBuff[1025], Name[1024];
    time_t rawtime;
    struct tm *info;
    char present_time[20];

    int connfd = *(int *)vargp;
    pthread_detach(pthread_self());
    free(vargp);
    
    while (1) {
        memset(recvBuff, 0, sizeof(recvBuff));
        read(connfd, recvBuff, sizeof(recvBuff) - 1);
        
        // leave the chatroom
        if (!strlen(recvBuff))
            break;

        strcpy(Name, strtok(recvBuff, ":"));
        int idx = -1;
        for (int i = 0; i < TotalPeople; ++i) {
            if (!strcmp(people[i].name, Name))
                idx = i;
        }
        if (idx == -1) {
            sem_wait(&mutex_TotalPeople);
            idx = TotalPeople;
            TotalPeople++;
            sem_post(&mutex_TotalPeople);
            strcpy(people[idx].name, Name);
            people[idx].chatCnt = chatCnt_global;
        }
    
        if (strlen(recvBuff + strlen(Name) + 1) > 0) {
            time(&rawtime);
            info = localtime(&rawtime);
            strftime(present_time, 20, "%H:%M:%S", info);
            sem_wait(&mutex_chatCnt_global);
            strcpy(chatHistory[chatCnt_global].message, recvBuff + strlen(Name) + 1);
            strcpy(chatHistory[chatCnt_global].name, Name);
            chatHistory[chatCnt_global].isLeaveMessage = false;
            strcpy(chatHistory[chatCnt_global].time, present_time);
            chatCnt_global = (chatCnt_global + 1) & (CHATSIZE - 1);
            sem_post(&mutex_chatCnt_global);
        }
        

        if (chatCnt_global != people[idx].chatCnt) {
            memset(sendBuff, 0, sizeof(sendBuff)); 
            for (; people[idx].chatCnt != chatCnt_global; people[idx].chatCnt = (people[idx].chatCnt + 1) & (CHATSIZE - 1)) {
                if (!strcmp(Name,chatHistory[people[idx].chatCnt].name)) {
                    strcat(sendBuff, "                                                               ");
                    strcat(sendBuff, chatHistory[people[idx].chatCnt].name);
                    strcat(sendBuff, "\n");
                    strcat(sendBuff, "                                                     ");
                    strcat(sendBuff, chatHistory[people[idx].chatCnt].message);
                    strcat(sendBuff, "(");
                    strcat(sendBuff, chatHistory[people[idx].chatCnt].time);
                    strcat(sendBuff, ")\n");
                } else {
                    strcat(sendBuff, chatHistory[people[idx].chatCnt].name);
                    strcat(sendBuff, "\n");
                    for (int i = 0; i < strlen(chatHistory[people[idx].chatCnt].name); ++i)
                        strcat(sendBuff, " ");
                    strcat(sendBuff, chatHistory[people[idx].chatCnt].message);
                    if (!chatHistory[people[idx].chatCnt].isLeaveMessage) {
                        strcat(sendBuff, "(");
                        strcat(sendBuff, chatHistory[people[idx].chatCnt].time);
                        strcat(sendBuff, ")\n");
                    } else
                        strcat(sendBuff, "\n");
                }
            }
            write(connfd, sendBuff, strlen(sendBuff) + 1);
        } 
        else {
            write(connfd, "\0", 1);
        }

        //printf("%s: %d(chatCnt) %d(global chatCnt)\n", Name, people[idx].chatCnt, chatCnt_global);
    }
    
    time(&rawtime);
    info = localtime(&rawtime);
    strftime(present_time, 20, "%H:%M:%S", info);
    memset(sendBuff, 0, sizeof(sendBuff)); 
    sprintf(sendBuff, "                    %s leave the chatroom(%s)                    ", Name, present_time);
    sem_wait(&mutex_chatCnt_global);
    strcpy(chatHistory[chatCnt_global].message, sendBuff);
    chatHistory[chatCnt_global].isLeaveMessage = true;
    strcpy(chatHistory[chatCnt_global].time, present_time);
    chatCnt_global = (chatCnt_global + 1) & (CHATSIZE - 1);
    sem_post(&mutex_chatCnt_global);
            
    close(connfd);

    return NULL;
}
