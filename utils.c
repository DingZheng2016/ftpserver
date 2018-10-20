#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "utils.h"

int deal_with_parameters(int* port, char* root_dir, int argc, char** argv){
    if(argc == 1)
        return 0;
    if(argc != 3 || argc != 5)
        return -1;
    //todo
    return 0;
}

int send_message(struct Client* c){
    write(c->sock, c->message, strlen(c->message));
    return 0;
}

int over_connections(int fd){
    char *message = "421 sorry, but the server is busy, please try again later.\r\n";
    write(fd, message, strlen(message));
    return 0;
}

int processing_command(char* s, char* cm, char* ms){
    sscanf(s, "%s %s",cm ,ms);
    return 0;
}

void *transfer_file(void* cv){
    printf("before file transfer.\n");
    
    struct Client* c = cv;

    char buffer[8192];
    c->filefd = open(c->filename,  O_RDONLY);

    while (1) {
        int bytes_read = read(c->filefd, buffer, sizeof(buffer));

        if (bytes_read == 0)
            break;

        if (bytes_read < 0) {
            perror("read error");
            return NULL;
        }

        void *p = buffer;
        while (bytes_read > 0) {
            int bytes_written = write(c->sockfd, p, bytes_read);
            if (bytes_written <= 0) {
                return NULL;
            }
            bytes_read -= bytes_written;
            p += bytes_written;
        }
    }
    c->message = "226 Transfer complete.\r\n";
    send_message(c);
    close(c->sockfd);
    return NULL;
}

int get_random_port(){
    srand((unsigned)(time(NULL)));
    return rand() % (65536 - 20000) + 20000;
}

int get_local_ip(int *h1, int* h2, int* h3, int* h4){
    *h1 = 127;
    *h2 = 0;
    *h3 = 0;
    *h4 = 1;
    return 0;
}