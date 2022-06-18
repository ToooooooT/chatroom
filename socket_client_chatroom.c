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
#include <semaphore.h>

#define SCREEN_WIDTH 70

enum {SECS_TO_SLEEP = 0, NSEC_TO_SLEEP = 125};

bool toQuit = false;

int main(int argc, char *argv[])
{
    int sockfd = 0, nameLen = strlen(argv[2]), messageLen;
    char recvBuff[1024], message[1024], name[12];
    struct sockaddr_in serv_addr;

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
    sprintf(message, "%s:write", argv[2]);
    write(sockfd, message, strlen(argv[2]));
    
    while (!toQuit) {
        memset(recvBuff, 0, sizeof(recvBuff));
        read(sockfd, recvBuff, sizeof(recvBuff));
        write(sockfd, "1", 2);
        if (!strncmp(recvBuff, "server:", 7) && !strncmp(recvBuff + 7, argv[2], nameLen))
            break;

        strcpy(name, strtok(recvBuff, ":"));
        messageLen = strlen(recvBuff + strlen(name) + 1) - 10;
        memset(message, 0, 1024);
        strncpy(message, recvBuff + strlen(name) + 1, messageLen);
        
        if (!strcmp(name, "server")) {
            int half = (SCREEN_WIDTH - messageLen) / 2;
            for (int i = 0; i < half; ++i)
                printf(" ");
            printf("%s", message);
            for (int i = 0; i < half; ++i)
                printf(" ");
            printf("\n");
        } else if (strcmp(argv[2], name)) {
            printf("%s\n", name);
            printf("'");
            for (int i = 0; i < messageLen + 2; ++i)
                printf("\"");
            printf(".\n");

            printf("| %s |\n", message);

            printf("'");
            for (int i = 0; i < messageLen + 2; ++i)
                printf("-");
            printf("'%s\n\n", recvBuff + strlen(name) + messageLen + 1);
        } else {
            for (int i = 0; i < SCREEN_WIDTH - strlen(name); ++i)
                printf(" ");
            printf("%s\n", name);
            
            for (int i = 0; i < SCREEN_WIDTH - messageLen - 4; ++i)
                printf(" ");
            printf(".");
            for (int i = 0; i < messageLen + 2; ++i)
                printf("-");
            printf("'\n");

            for (int i = 0; i < SCREEN_WIDTH - messageLen - 4; ++i)
                printf(" ");
            printf("| %s |\n", message);

            for (int i = 0; i < SCREEN_WIDTH - messageLen - 4 - 10; ++i)
                printf(" ");
            printf("%s", recvBuff + strlen(name) + messageLen + 1);
            printf("'");
            for (int i = 0; i < messageLen + 2; ++i)
                printf("-");
            printf("'\n\n");
        }
    }
    write(sockfd, "", 2);
    close(sockfd);

    return 0;
}
