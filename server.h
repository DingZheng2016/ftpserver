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
    struct sockaddr_in addr;
    socklen_t len;

    char username[200];
    int login;
};

void init_server(struct Server*, int, char*);

void run_server();

#endif