#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "utils.h"

int deal_with_parameters(int* port, char* root_dir, int argc, char** argv){
    if(argc == 1)
        return 0;
    if(argc != 3 || argc != 5)
        return -1;
    //todo
    return 0;
}

int send_message(struct Client* c, int type){
    switch(type){
        case 0:{
            char message[] = "220 Anonymous FTP server ready.\r\n";
            write(c->sock, message, strlen(message));
            break;
        }case 1:{
            char message[] = "331 Guest login ok, send your complete e-mail address as password.\r\n";
            printf("%s\n", message);
            write(c->sock, message, strlen(message));
            break;
        }case 2:{
            char message[] = "230 login successfully.\r\n";
            printf("%s\n", message);
            write(c->sock, message, strlen(message));
            break;
        }
    }
    return 0;
}

int processing_command(char* s, char* cm, char* ms){
    sscanf(s, "%s %s",cm ,ms);
    return 0;
}

int get_random_port(){
    srand((unsigned)(time(NULL)));
    return rand() % (65536 - 20000) + 20000;
}
