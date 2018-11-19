#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>
#include <arpa/inet.h>

#include "server.h"
#include "utils.h"
#include "handler.h"


void init_server(struct Server* sv, int port, char* root_dir){

    int sock;
    strcpy(sv->root_dir, root_dir);

    if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
        return ;
    }

    sv->sock = sock;

    memset(&sv->addr, 0, sizeof(sv->addr));
    sv->addr.sin_family = AF_INET;
    sv->addr.sin_port = htons(port);
    sv->addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sock, (struct sockaddr*)&sv->addr, sizeof(sv->addr)) == -1){
        return ;
    }

    if(listen(sock, 10) == -1){
        return ;
    }
}

void run_server(struct Server *sv){

    int max_sock;
    int cs;
    int act;
    int n;
    int read_error;
    int p;
    char sentence[8192];
    
    fd_set readfds;
    struct Client* clients[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; ++i){
        clients[i] = (struct Client*)malloc(sizeof(struct Client));
        init_client(clients[i]);
    }

    while(1){

        FD_ZERO(&readfds);
        FD_SET(sv->sock, &readfds);

        max_sock = sv->sock;

        for(int i = 0; i < MAX_CLIENTS; ++i){

            cs = clients[i]->sock;

            if(cs >= 0)
                FD_SET(cs, &readfds);
            if(cs > max_sock)
                max_sock = cs;
        }

        act = select(max_sock + 1, &readfds, NULL, NULL, NULL);

        if(act < 0 && errno != EINTR){
            continue;
        }

        if(FD_ISSET(sv->sock, &readfds)){
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);

            if((cs = accept(sv->sock, (struct sockaddr*)&client_addr, &client_len)) == -1){
                continue;
            }

            int i = 0;
            for(; i < MAX_CLIENTS && clients[i]->sock != -1; ++i);

            if(i >= MAX_CLIENTS){
                over_connections(cs);
                continue;
            }
            
            clients[i]->sock = cs;
            clients[i]->addr = client_addr;
            clients[i]->len = client_len;
            clients[i]->root_dir = sv->root_dir;
            strcpy(clients[i]->dir, "/");

            clients[i]->message = "220 Anonymous FTP server ready.\r\n";
            send_message(clients[i]);

        }else{

            int i = 0;
            for(; i < MAX_CLIENTS && !FD_ISSET(clients[i]->sock, &readfds); ++i);
            
            if(i >= MAX_CLIENTS) {
                continue;
            }

            cs = clients[i]->sock;
            read_error = 0;
            p = 0;            

            while(1){
                n = read(cs, sentence + p, 8191 - p);
                if(n < 0){
                    read_error = 1;
                    break;
                }else if(n == 0){
                    break;
                }else{
                    p += n;
                    if(sentence[p - 1] == '\n')
                        break;
                }
            }

            if(read_error || p == 0){
                close(cs);
                init_client(clients[i]);
                continue;
            }

            sentence[p - 1] = '\0';

            char command[20];
            char message[200];

            processing_command(sentence, command, message);
            handle_command(clients[i], command, message);

        }
    }
}

void init_client(struct Client* c){
    c->sock = -1;
    c->sockfd = -1;
    c->socklfd = -1;
    c->filefd = -1;
    c->login = 0;
    c->type = 0;
    c->skip_bytes = 0;
    memset(c->rn_be, 0, sizeof(c->rn_be));
}
