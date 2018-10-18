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
        perror("socket error");
        return ;
    }

    sv->sock = sock;

    memset(&sv->addr, 0, sizeof(sv->addr));
    sv->addr.sin_family = AF_INET;
    sv->addr.sin_port = htons(port);
    sv->addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sock, (struct sockaddr*)&sv->addr, sizeof(sv->addr)) == -1){
        perror("bind error");
        return ;
    }

    if(listen(sock, 10) == -1){
		perror("listen error");
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
        clients[i]->sock = -1;
    }

    while(1){

        FD_ZERO(&readfds);
        FD_SET(sv->sock, &readfds);

        max_sock = sv->sock;
        //printf("%d\n", sv->sock);

        for(int i = 0; i < MAX_CLIENTS; ++i){

            cs = clients[i]->sock;

            if(cs >= 0)
                FD_SET(cs, &readfds);
            if(cs > max_sock)
                max_sock = cs;
        }
        //printf("before select\n");
        act = select(max_sock + 1, &readfds, NULL, NULL, NULL);

        printf("select %d\n", act);

        if(act < 0 && errno != EINTR){
            perror("select error");
            continue;
        }

        if(FD_ISSET(sv->sock, &readfds)){
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);

            if((cs = accept(sv->sock, (struct sockaddr*)&client_addr, &client_len)) == -1){
                perror("accept error");
                continue;
            }

            printf("cs:%d\n", cs);

            int i = 0;
            for(; i < MAX_CLIENTS && clients[i]->sock != -1; ++i);

            if(i >= MAX_CLIENTS)
                continue;
            
            clients[i]->sock = cs;
            clients[i]->addr = client_addr;
            clients[i]->len = client_len;

            printf("sock:%d\n", clients[i]->sock);

            send_message(clients[i], 0);

        }else{

            int i = 0;
            for(; i < MAX_CLIENTS && !FD_ISSET(clients[i]->sock, &readfds); ++i);
            
            if(i >= MAX_CLIENTS) {
                printf("read from none.\n");
                continue;
            }

            cs = clients[i]->sock;
            printf("clientsock: %d\n", cs);
            read_error = 0;
            p = 0;            

            while(1){
                n = read(cs, sentence + p, 8191 - p);
                if(n < 0){
                    perror("read error");
                    close(cs);
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

            if(read_error)
                continue;

            sentence[p - 1] = '\0';
		    //len = p - 1;
		
		    //for(p = 0; p < len; p++)
			//    sentence[p] = toupper(sentence[p]);
            printf("%s\n", sentence);
            char command[20];
            char message[200];
            processing_command(sentence, command, message);
            printf("%s\n", command);
            printf("%s\n", message);

            handle_command(clients[i], command, message);

        }
    }
}
