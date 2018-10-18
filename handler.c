#include <string.h>

#include "handler.h"
#include "utils.h"

int handle_command(struct Client* c, char* command, char* message){
    if(strcmp(command, "USER") == 0)
        handle_USER(c, message);
    else if(strcmp(command, "PASS") == 0)
        handle_PASS(c, message);
    return 0;
}

int handle_USER(struct Client* c, char* username){
    if(strcmp(username, "anonymous") == 0)
        send_message(c, 1);
    return 0;
}

int handle_PASS(struct Client* c, char* password){
    send_message(c, 2);
    return 0;
}