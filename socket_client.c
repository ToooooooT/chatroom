#include <stdio.h>
#include <string.h>

int main (int argc, char *argv[]) {
    char command[512];
    sprintf(command, "gnome-terminal --title=\"%s's read\" --geometry=70x3+10+620 -- ./socket_client_read 127.0.0.1 %s", argv[1], argv[1]);
    FILE *fp1 = popen(command, "w");

    sprintf(command, "gnome-terminal --title=\"%s's chatroom\" --geometry=70x30+10 -- ./socket_client_chatroom 127.0.0.1 %s", argv[1], argv[1]);
    FILE *fp2 = popen(command, "w");

    pclose(fp1);
    pclose(fp2);

    return 0;
}

