#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_CLIENTS 30

struct Server{
    int sock;
    char root_dir[100];
    struct sockaddr_in addr;
};

struct Client{
    int sock;
    int sockfd; //file sock
    int socklfd; //listen file sock
    struct sockaddr_in addr;
    socklen_t len;

    char *message;
    char *root_dir;
    char dir[200];
    char filename[200];
    char username[200];
    char password[200];
    char rn_be[200];
    int login;// 0 for not login, 1 for login
    int type;// 1 for I, 10 for A
    int filefd;
    int mode; // 1 for PORT, 2 for PASV, 0 for none
    int skip_bytes;
};

void init_server(struct Server*, int, char*);

void run_server();

void init_client(struct Client*);

#endif