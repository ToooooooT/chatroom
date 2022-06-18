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
#include <time.h>

enum {SECS_TO_SLEEP = 0, NSEC_TO_SLEEP = 125};
enum {quit = 1, chat = 2, sticker = 3};

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

    struct timespec remaining, request = {SECS_TO_SLEEP, NSEC_TO_SLEEP};

    while (!toQuit) {
        memset(message, 0, sizeof(message)); 
        fgets(message + 1, 1024, stdin);
        popen("reset", "w");
        message[strlen(message + 1)] = 0;
        message[0] = chat;
        if (message[1] == '/') {
            if (!strcmp(message + 1, "/quit")) {
                toQuit = true;
                message[0] = quit;
            } else if (!strncmp(message + 1, "/sticker ", 9)) {
                message[0] = sticker;
                message[1] = atoi(message + 10);
                message[2] = 0;
            } else {
                nanosleep(&request, &remaining);
                continue;
            }
        }
        write(sockfd, message, strlen(message) + 1);
        
        nanosleep(&request, &remaining);
    }
    close(sockfd);


    return 0;
}
