#include <stdio.h>
#include <string.h>

int main (int argc, char *argv[]) {
    char command[512];
    // command to execute in current terminal to open new terminal window called name's read to execute a new process to recieve user's message
    sprintf(command, "gnome-terminal --title=\"%s's read\" --geometry=70x3+10+620 -- ./socket_client_read 127.0.0.1 %s", argv[1], argv[1]);
    FILE *fp1 = popen(command, "w");

    // command to execute in current terminal to open new terminal window called name's chatroom to execute a new process to print message send to chatroom
    sprintf(command, "gnome-terminal --title=\"%s's chatroom\" --geometry=70x30+10 -- ./socket_client_chatroom 127.0.0.1 %s", argv[1], argv[1]);
    FILE *fp2 = popen(command, "w");

    pclose(fp1);
    pclose(fp2);

    return 0;
}

