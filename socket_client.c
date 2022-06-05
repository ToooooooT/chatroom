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

int main(int argc, char *argv[])
{
    int sockfd = 0, n = 0;
    char recvBuff[1024], op[10], message[1024], sendBuff[2050];
    struct sockaddr_in serv_addr; 
    bool haveEnteredName = false;

    if(argc != 3)
    {
        printf("\n Usage: %s <ip of server> \n",argv[0]);
        return 1;
    }
    
    memset(recvBuff, 0 ,sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 
    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000); 

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {   
        printf("\n inet_pton error occured\n");
        return 1;
    }

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }

    while (1) {
        if (!haveEnteredName) {             
            memset(message, 0, 1024);
            sprintf(message, "%s:", argv[2]);
            write(sockfd, message, strlen(argv[2]));
            haveEnteredName = true;
        } else {
        
            printf(">>> (op) ");
            scanf("%s", op);

            if (op[0] == 'q') {
                printf("%s leave the chatroom\n", argv[2]);
                break;
            }
            else if (op[0] == 'w') {
                // write
                memset(message, 0, 1024);
                printf(">>> (send message) ");
                scanf("%s", message);            
                sprintf(sendBuff, "%s:%s", argv[2], message);
                write(sockfd, sendBuff, strlen(sendBuff) + 1);
            }
            else {
                memset(message, 0, 1024);
                sprintf(message, "%s:", argv[2]);
                write(sockfd, message, strlen(message) + 1);
            }
        }

        // read history
        sleep(1);
        n = read(sockfd, recvBuff, sizeof(recvBuff) - 1);
        if (strlen(recvBuff))
            printf("%s", recvBuff);
            /*
            if(fputs(recvBuff, stdout) == EOF)
            {
                printf("\n Error : Fputs error\n");
            }
            */
    }

    if(n < 0)
    {
        printf("\n Read error \n");
    } 

    return 0;
}
