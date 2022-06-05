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

typedef struct message {
    char name[1024];
    char message[1024];
} message_t;

typedef struct person {
    char name[1024];
    int chatCnt;
} person_t;

int TotalPeople = 0;
person_t people[1024];
message_t chatHistory[1025];
int chatCnt_global = 0;

int listenfd = 0;

void *thread(void *vargp);

int main(int argc, char *argv[])
{
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

    int connfd = *(int *)vargp;
    pthread_detach(pthread_self());
    free(vargp);

    memset(recvBuff, 0, sizeof(recvBuff));
    read(connfd, recvBuff, sizeof(recvBuff) - 1);
    strcpy(Name, strtok(recvBuff, ":"));
    int idx = -1;
    for (int i = 0; i < TotalPeople; ++i) {
        if (!strcmp(people[i].name, Name))
            idx = i;
    }
    if (idx == -1) {
        idx = TotalPeople;
        TotalPeople++;
        strcpy(people[idx].name, Name);
        people[idx].chatCnt = chatCnt_global;
    }
    
    if (strlen(recvBuff + strlen(Name) + 1) > 0) {
        strcpy(chatHistory[chatCnt_global].message, recvBuff + strlen(Name) + 1);
        strcpy(chatHistory[chatCnt_global].name, Name);
        chatCnt_global++;
    }

    if (chatCnt_global > people[idx].chatCnt) {
        memset(sendBuff, 0, sizeof(sendBuff)); 
        for (; people[idx].chatCnt < chatCnt_global; people[idx].chatCnt++) {
            if (!strcmp(Name,chatHistory[people[idx].chatCnt].name)) {
                strcat(sendBuff, "                                                               ");
                strcat(sendBuff, ": "); 
                strcat(sendBuff, chatHistory[people[idx].chatCnt].name);
                strcat(sendBuff, "\n");
                strcat(sendBuff, "                                                     ");
                strcat(sendBuff, chatHistory[people[idx].chatCnt].message);
                strcat(sendBuff, "\n");
            } else {
                strcat(sendBuff, chatHistory[people[idx].chatCnt].name);
                strcat(sendBuff, ":\n");
                for (int i = 0; i < strlen(chatHistory[people[idx].chatCnt].name); ++i)
                    strcat(sendBuff, " ");
                strcat(sendBuff, chatHistory[people[idx].chatCnt].message);
                strcat(sendBuff, "\n");
            }
        }
        write(connfd, sendBuff, strlen(sendBuff));
    } 
    else {
        write(connfd, "\0", 1);
    }
            
    close(connfd);

    return NULL;
}
