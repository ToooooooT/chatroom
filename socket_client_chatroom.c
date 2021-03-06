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
#include "sticker.h"

#define SCREEN_WIDTH 70
#define PRINT_CHAR(len, c)              \
    do {                                \
        for (int i = 0; i < (len); ++i) \
            putchar(c);                 \
    } while(0)                          \

enum {SECS_TO_SLEEP = 0, NSEC_TO_SLEEP = 125};
enum {quit = 1, chat = 2, sticker = 3};

int main(int argc, char *argv[])
{
    int sockfd = 0, nameLen = strlen(argv[2]), messageLen;
    char recvBuff[1024], message[1024], name[12], stickerID;
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

    // send the first message to server to let server know who is enter in the chatroom and create a new thread for this client
    memset(message, 0, 1024);
    sprintf(message, "%s:write", argv[2]);
    write(sockfd, message, strlen(argv[2]));

    PRINT_CHAR(((SCREEN_WIDTH - 38) / 2), ' ');
    printf("contributors: stupidrara and ToooooooT\n");
    
    while (1) {
        memset(recvBuff, 0, sizeof(recvBuff));
        read(sockfd, recvBuff, sizeof(recvBuff));
        write(sockfd, "1", 2); // send 2 byte message to notice serever that this process is still connect

        if (recvBuff[0] == quit && !strncmp(recvBuff + 8, argv[2], nameLen)) {
            // quit this process when first command is quit and the name is match with this user name
            break;
        }

        if (recvBuff[0] == sticker) {
            // sticker command
            strcpy(name, strtok(recvBuff + 1, ":")); // copy name who send the sticker

            stickerID = recvBuff[strlen(name) + 2];
            int stickerWidth = match_sticker(message, stickerID);
            if (stickerWidth <= 0) {
                // invalid stickerID
                continue;
            }

            char *p = message;
            if (strcmp(argv[2], name)) {
                // other person send the sticker
                printf("%s\n", name);
                while (*(p + stickerWidth)) {
                    for (int i = 0; i < stickerWidth; ++i, ++p)
                        printf("%c", *p);
                    printf("\n");
                }
                for (int i = 0; i < stickerWidth; ++i, ++p)
                    printf("%c", *p);
                printf("%s\n\n", recvBuff + strlen(name) + 3);
            } else {
                // user send the sticker
                PRINT_CHAR(SCREEN_WIDTH - strlen(name), ' ');
                printf("%s\n", name);
                while (*(p + stickerWidth)) {
                    PRINT_CHAR(SCREEN_WIDTH - stickerWidth, ' ');
                    for (int i = 0; i < stickerWidth; ++i, ++p)
                        printf("%c", *p);
                    printf("\n");
                }
                PRINT_CHAR(SCREEN_WIDTH - stickerWidth - 10, ' ');
                printf("%s", recvBuff + strlen(name) + 3);
                for (int i = 0; i < stickerWidth; ++i, ++p)
                    printf("%c", *p);
                printf("\n\n");
            }
        } else {
            // message command
            strcpy(name, strtok(recvBuff + 1, ":")); // copy name who send the sticker
            messageLen = strlen(recvBuff + strlen(name) + 2) - 10;
            memset(message, 0, 1024);
            strncpy(message, recvBuff + strlen(name) + 2, messageLen);
        
            if (!strcmp(name, "server")) {
                // server's message
                int half = (SCREEN_WIDTH - messageLen) / 2;
                PRINT_CHAR(half, ' ');
                printf("%s", message);
                PRINT_CHAR(half, ' ');
                printf("\n");
            } else if (strcmp(argv[2], name)) {
                // other person's message
                printf("%s\n", name);
                printf("'");
                PRINT_CHAR(messageLen + 2, '\"');
                printf(".\n");

                printf("| %s |\n", message);

                printf("'");
                PRINT_CHAR(messageLen + 2, '-');
                printf("'%s\n\n", recvBuff + strlen(name) + messageLen + 2);
            } else {
                // user's message
                PRINT_CHAR(SCREEN_WIDTH - strlen(name), ' ');
                printf("%s\n", name);
                PRINT_CHAR(SCREEN_WIDTH - messageLen - 4, ' ');
                printf(".");
                PRINT_CHAR(messageLen + 2, '-');
                printf("'\n");

                PRINT_CHAR(SCREEN_WIDTH - messageLen - 4, ' ');
                printf("| %s |\n", message);

                PRINT_CHAR(SCREEN_WIDTH - messageLen - 4 - 10, ' ');
                printf("%s", recvBuff + strlen(name) + messageLen + 2);
                printf("'");
                PRINT_CHAR(messageLen + 2, '-');
                printf("'\n\n");
            }
        }
    }
    write(sockfd, "", 2);
    close(sockfd);

    return 0;
}
