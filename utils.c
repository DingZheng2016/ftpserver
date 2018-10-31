#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#include "utils.h"

int deal_with_parameters(int* port, char* root_dir, int argc, char** argv){
    if(argc == 1)
        return 0;
    
    if(argc != 3 && argc != 5)
        return -1;
    
    if(argc == 3){
        if(strcmp(argv[1], "-port") == 0)
            *port = atoi(argv[2]);
        else if(strcmp(argv[2], "-root") == 0)
            strcpy(root_dir, argv[2]);
    }

    if(argc == 5){
        for(int i = 0; i < 2; ++i)
            if(strcmp(argv[i * 2 + 1], "-port") == 0)
                *port = atoi(argv[i * 2 + 2]);
            else if(strcmp(argv[i * 2 + 1], "-root") == 0)
                strcpy(root_dir, argv[i * 2 + 2]);
    }

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
    char file_path[200];
    strcpy(file_path, c->root_dir);
    strcat(file_path, c->dir);
    strcat(file_path, c->filename);
    printf("%s\n", file_path);
    c->filefd = open(file_path,  O_RDONLY);

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
    c->mode = 0;
    return NULL;
}

void *store_file(void *cv){
    struct Client* c = cv;

    char file_path[200];
    strcpy(file_path, c->root_dir);
    strcat(file_path, c->dir);
    strcat(file_path, c->filename);

    int fp;
    fp = open(file_path, O_WRONLY | O_CREAT , S_IRWXG | S_IRWXO | S_IRWXU);

    char buffer[8192];
    int p = 0;
    printf("before store.\n");

    while (1) {
        int bytes_read = read(c->sockfd, buffer + p, 8191 - p);
        if (bytes_read < 0) {
            perror("read error.");
            close(c->sockfd);
            break;
        } else if (bytes_read == 0) {
            break;
        }
        int file_state = write(fp, buffer, bytes_read);
        if(file_state == 0){
            printf("Error while writing the file!");
            break;
        }
    }
    close(fp);
    printf("store completed.\n");

    c->message = "226 Transfer complete.\r\n";
    send_message(c);
    close(c->sockfd);
    c->mode = 0;
    return NULL;
}

void *transfer_list(void *cv){
    struct Client* c = cv;
    char com[400];
    sprintf(com, "ls -l %s%s", c->root_dir, c->dir);
    FILE *fs = popen(com, "r");
    char buffer[8192];
    while(fgets(buffer, 8191, fs)){
        int p = 0;
        int len = strlen(buffer);
        while (p < len) {
            int n = write(c->sockfd, buffer + p, len - p);
            if (n < 0) {
                perror("write error.");
                close(c->sockfd);
                return NULL;
            } else {
                p += n;
            }
        }
    }

    c->message = "226 Transfer complete.\r\n";
    send_message(c);
    close(c->sockfd);
    c->mode = 0;
    return NULL;
}

int get_random_port(){
    srand((unsigned)(time(NULL)));
    return rand() % (65536 - 20000) + 20000;
}

int get_local_ip(int sock, int *h1, int* h2, int* h3, int* h4){
    socklen_t addr_size = sizeof(struct sockaddr_in);
    struct sockaddr_in addr;
    getsockname(sock, (struct sockaddr *)&addr, &addr_size);
 
    char* host = inet_ntoa(addr.sin_addr);
    sscanf(host,"%d.%d.%d.%d", h1, h2, h3, h4);
    return 0;
}

int checkuserinfo(char* username, char* password){
    FILE* fp = fopen("userinfo.txt", "r");
    char user[200], pass[200];
    while(fscanf(fp, "%s\n", user) != EOF){
        printf("%s\n", user);
        fscanf(fp, "%s\n", pass);
        if(strcmp(user, username) == 0 && strcmp(pass, password) == 0)
            return 0;
    }
    return -1;
}
