#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>

#include "handler.h"
#include "utils.h"

// error message
char* const message_unlogin = "530 Please log in with USER and PASS first.\r\n";
char* const message_unknowncommand = "500 Syntax error, command unrecognized.\r\n";
char* const message_modeundefined = "503 Bad sequence of commands.\r\n";
char* const message_rntowithoutrnfr = "503 Bad sequence of commands.\r\n";
char* const message_dataconnectionerror = "425 Can't open data connection.\r\n";
char* const message_connectionerror = "426 TCP connection was established but then broken by the client or by network failure.\r\n";
char* const message_unsupportedtype = "501 Unsupported type.\r\n";
char* const message_alreadylogin = "530 Can't change from guest user.\r\n";
char* const message_errorlogin = "530 Login incorrect.\r\n";

int handle_command(struct Client* c, char* command, char* message){
    if(strcmp(command, "USER") == 0)
        handle_USER(c, message);        
    else if(strcmp(command, "PASS") == 0)
        handle_PASS(c, message);
    else if(strcmp(command, "SYST") == 0)
        handle_SYST(c);
    else if(strcmp(command, "TYPE") == 0)
        handle_TYPE(c, message);
    else if(strcmp(command, "QUIT") == 0)
        handle_QUIT(c);
    else if(strcmp(command, "PORT") == 0)
        handle_PORT(c, message);
    else if(strcmp(command, "RETR") == 0)
        handle_RETR(c, message);
    else if(strcmp(command, "PASV") == 0)
        handle_PASV(c);
    else if(strcmp(command, "STOR") == 0)
        handle_STOR(c, message);
    else if(strcmp(command, "CWD") == 0)
        handle_CWD(c, message);
    else if(strcmp(command, "PWD") == 0)
        handle_PWD(c);
    else if(strcmp(command, "MKD") == 0)
        handle_MKD(c, message);
    else if(strcmp(command, "LIST") == 0)
        handle_LIST(c);
    else if(strcmp(command, "RMD") == 0)
        handle_RMD(c, message);
    else if(strcmp(command, "RNFR") == 0)
        handle_RNFR(c, message);
    else if(strcmp(command, "RNTO") == 0)
        handle_RNTO(c, message);
    else{
        c->message = message_unknowncommand;
        send_message(c);
    }
    return 0;
}

int handle_USER(struct Client* c, char* username){
    /*
     * Handle USER command.
     * If already logged in, return 530.
     * Don't check the username.
     * Just return different messages according to whether username is anonymous.
     * Return 331 if not logged in.
     */
    if(c->login == 1){
        c->message = message_alreadylogin;
        send_message(c);
        return 1;      
    }

    if(strcmp(username, "anonymous") == 0)
        c->message = "331 Guest login ok, send your complete e-mail address as password.\r\n";
    else
        c->message = "331 Please specify the password.\r\n";
    
    send_message(c);
    strcpy(c->username, username);
    return 0;
}

int handle_PASS(struct Client* c, char* password){
    /*
     * Handle PASS command.
     * If already logged in, return 230.
     * Check the username and the password.
     * Return 530 if error else return 230.
     */
    if(c->login == 1){
        c->message = "230 Already logged in.\r\n";
        send_message(c);
        return 1;
    }

    if(strcmp(c->username, "anonymous") == 0){
        c->message = "230 login successfully.\r\n";
        send_message(c);
        strcpy(c->password, password);
        c->login = 1;
    }else if(checkuserinfo(c->username, password) == 0){
        c->message = "230 login successfully.\r\n";
        send_message(c);
        strcpy(c->password, password);
        c->login = 1;
    }else{
        c->message = message_errorlogin;
        send_message(c);
        return 1;
    }
    return 0;
}

int handle_SYST(struct Client* c){
    /*
     * Handle SYST command.
     * Return 215.
     */
    if(!c->login){
        c->message = message_unlogin;
        send_message(c);
        return 1;
    }

    c->message = "215 UNIX Type: L8\r\n";
    send_message(c);
    return 0;
}

int handle_TYPE(struct Client* c, char* type){
    /*
     * Handle TYPE command.
     * I for RETR and STOR, A for LIST.
     * Return 501 if type is unsupported.
     * Return 200 if successful.
     */
    if(!c->login){
        c->message = message_unlogin;
        send_message(c);
        return 1;
    }

    if(strcmp(type, "I") == 0){
        c->message = "200 Type set to I.\r\n";
        send_message(c);
        c->type = 1;
    }else if(strcmp(type, "A") == 0){
        c->message = "200 Type set to A.\r\n";
        send_message(c);
        c->type = 10;
    }else{
        c->message = message_unsupportedtype;
        send_message(c);
        return 1;
    }
    return 0;
}

int handle_QUIT(struct Client* c){
    /*
     * Handle QUIT command.
     * Return 221.
     */
    c->message = "221 Goodbye!\r\n";
    send_message(c);
    close(c->sock);
    init_client(c);
    return 0;
}

int handle_PORT(struct Client* c, char* s){
    /*
     * HANDLE PORT command.
     * Return 425 if data connection can't establish.
     * Return 200 if successful.
     */

    if(!c->login){
        c->message = message_unlogin;
        send_message(c);
        return 1;
    }

    int h1, h2, h3, h4, p1, p2;
    sscanf(s, "%d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4 ,&p1, &p2);
    memset(&(c->addr), 0, sizeof(c->addr));
    c->addr.sin_port = htons(p1 * 256 + p2);
    c->addr.sin_family = AF_INET;
    char ip[20];
    sprintf(ip, "%d.%d.%d.%d", h1, h2, h3, h4);
    if(inet_pton(AF_INET, ip, &(c->addr.sin_addr)) != 1){
        c->message = message_dataconnectionerror;
        send_message(c);
        return 1;
    }

    if ((c->sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        c->message = message_dataconnectionerror;
        send_message(c);
		return 1;
	}
    c->mode = 1;
    c->message = "200 PORT command successful.\r\n";
    send_message(c);
    return 0;
}

int handle_RETR(struct Client* c, char* filename){
    /*
     * Handle RETR command.
     * Return 425 if data connection can't establish.
     * Return 503 if mode undefined.
     * Return 150 before transfer starts.
     * Return 226 after transfer completes.
     * Open a new thread to transfer file to avoid blocking other clients.
     */

    if(!c->login){
        c->message = message_unlogin;
        send_message(c);
        return 1;
    }

    if(c->mode == 1){

        if(connect(c->sockfd, (struct sockaddr*)&(c->addr), sizeof(c->addr)) == -1){
            c->message = message_dataconnectionerror;
            send_message(c);
            return 1;
        }
        struct stat st;
        stat(filename, &st);
        int size = st.st_size;

        char message[200];
        sprintf(message, "150 Opening BINARY mode data connection for %s (%d bytes).\r\n", filename, size);
        c->message = message;
        send_message(c);

        strcpy(c->filename, filename);
        pthread_t thread_id; 
        pthread_create(&thread_id, NULL, transfer_file, c);

    }else if(c->mode == 2){

        if((c->sockfd = accept(c->socklfd, NULL, NULL)) == -1){
            c->message = message_dataconnectionerror;
            send_message(c);
            return 1;
        }

        struct stat st;
        stat(filename, &st);
        int size = st.st_size;

        char message[200];
        sprintf(message, "150 Opening BINARY mode data connection for %s (%d bytes).\r\n", filename, size);
        c->message = message;
        send_message(c);

        strcpy(c->filename, filename);
        pthread_t thread_id; 
        pthread_create(&thread_id, NULL, transfer_file, c);


    }else{
        c->message = message_modeundefined;
        send_message(c);
        return 1;
    }
    return 0;
}

int handle_PASV(struct Client* c){
    /*
     * Handle PASV command.
     * Return 425 if data connection can't establish.
     * Return 227 if successful.
     */

    if(!c->login){
        c->message = message_unlogin;
        send_message(c);
        return 1;
    }

    int rand_port = get_random_port();
    int p1 = rand_port / 256;
    int p2 = rand_port % 256;
    int h1, h2, h3, h4;
    get_local_ip(c->sock, &h1, &h2, &h3, &h4);

    //sscanf(s, "%d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4 ,&p1, &p2);
    memset(&(c->addr), 0, sizeof(c->addr));
    c->addr.sin_port = htons(rand_port);
    c->addr.sin_family = AF_INET;
    char ip[20];
    sprintf(ip, "%d.%d.%d.%d", h1, h2, h3, h4);
    if(inet_pton(AF_INET, ip, &(c->addr.sin_addr)) != 1){
        c->message = message_dataconnectionerror;
        send_message(c);
        return 1;
    }

    if ((c->socklfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        c->message = message_dataconnectionerror;
        send_message(c);
		return 1;
	}

    if(bind(c->socklfd, (struct sockaddr*)&c->addr, sizeof(c->addr)) == -1){
        c->message = message_dataconnectionerror;
        send_message(c);
        return 1;
    }

    if(listen(c->socklfd, 1) == -1){
        c->message = message_dataconnectionerror;
        send_message(c);
        return 1;
    }

    c->mode = 2;

    char message[200];
    sprintf(message, "227 =%d,%d,%d,%d,%d,%d\r\n", h1, h2, h3, h4, p1, p2);
    c->message = message;
    send_message(c);
    return 0;
}

int handle_STOR(struct Client* c, char* dir){
    /*
     * Handle STOR command.
     * Return 425 if data connection can't establish.
     * Return 503 if mode undefined.
     * Return 150 before file transfer.
     * Return 226 after file transfer.
     * Open a new thread to transfer file avoiding other clients from being blocked.
     */

    if(!c->login){
        c->message = message_unlogin;
        send_message(c);
        return 1;
    }

    if(c->mode == 1){
        if(connect(c->sockfd, (struct sockaddr*)&(c->addr), sizeof(c->addr)) == -1){
            c->message = message_dataconnectionerror;
            send_message(c);
            return 1;
        }

        char message[200];
        sprintf(message, "150 Opening BINARY mode data connection for %s.\r\n", dir);
        c->message = message;
        send_message(c);

        strcpy(c->filename, dir);
        pthread_t thread_id; 
        pthread_create(&thread_id, NULL, store_file, c);

    }else if(c->mode == 2){

        if((c->sockfd = accept(c->socklfd, NULL, NULL)) == -1){
            c->message = message_dataconnectionerror;
            send_message(c);
            return 1;
        }

        char message[200];
        sprintf(message, "150 Opening BINARY mode data connection for %s.\r\n", dir);
        c->message = message;
        send_message(c);

        strcpy(c->filename, dir);
        pthread_t thread_id; 
        pthread_create(&thread_id, NULL, store_file, c);

    }else{
        c->message = message_modeundefined;
        send_message(c);
        return 1;
    }
    
    return 0;
}

int handle_CWD(struct Client* c, char* dir){
    /*
     * Handle CWD command.
     * Return 250 if Okay.
     * Return 550 if not found.
     * Store the new dir in c->dir if successful.
     */

    if(!c->login){
        c->message = message_unlogin;
        send_message(c);
        return 1;
    }

    if(dir[strlen(dir) - 1] != '/')
        strcat(dir, "/");

    char path[200];
    strcpy(path, c->root_dir);
    if(dir[0] == '/'){
        strcat(path, dir);
        DIR* d = opendir(path);
        if(d){
            c->message = "250 Okay.\r\n";
            send_message(c);
            strcpy(c->dir, dir);
        }else{
            c->message = "550 No such file or directory.\r\n";
            send_message(c);
            return 1;
        }
    }else{

        strcat(path, c->dir);
        strcat(path, dir);

        DIR* d = opendir(path);
        if(d){
            c->message = "250 Okay.\r\n";
            send_message(c);
            strcat(c->dir, dir);
        }else{
            c->message = "550 No such file or directory.\r\n";
            send_message(c);
            return 1;
        }
    }

    return 0;
}

int handle_PWD(struct Client* c){
    /*
     * Handle PWD command.
     * Return 257.
     */

    if(!c->login){
        c->message = message_unlogin;
        send_message(c);
        return 1;
    }

    char message[300];
    sprintf(message, "257 \"%s\" is current directory.\r\n", c->dir);
    c->message = message;
    send_message(c);

    return 0;
}

int handle_MKD(struct Client* c, char* dir){
    /*
     * Handle MKD command.
     * Return 250 if successful.
     * Return 550 if created failed.
     */

    if(!c->login){
        c->message = message_unlogin;
        send_message(c);
        return 1;
    }

    char path[200];
    strcpy(path, c->root_dir);
    strcat(path, c->dir);
    strcat(path, dir);

    char message[300];
    if(mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1){    
        sprintf(message, "550 \"%s\" created failed.\r\n", dir);
        c->message = message;
        send_message(c);
    }else{
        sprintf(message, "250 \"%s\" created successfully.\r\n", dir);
        c->message = message;
        send_message(c);
        return 1;
    }

    return 0;
}

int handle_LIST(struct Client *c){
    /*
     * Handle LIST command.
     * Return 425 if data connection can't establish.
     * Return 150 before LIST info transfer.
     * Return 226 after LIST info transfer.
     * Return 503 if mode undefined.
     * Open a new thread to transfer list info.
     */

    if(!c->login){
        c->message = message_unlogin;
        send_message(c);
        return 1;
    }

    if(c->mode == 1){

        if(connect(c->sockfd, (struct sockaddr*)&(c->addr), sizeof(c->addr)) == -1){
            c->message = message_dataconnectionerror;
            send_message(c);
            return 1;
        }

        char message[200];
        sprintf(message, "150 Opening data connection for contents of current directory.\r\n");
        c->message = message;
        send_message(c);

        pthread_t thread_id; 
        pthread_create(&thread_id, NULL, transfer_list, c);


    }else if(c->mode == 2){

        if((c->sockfd = accept(c->socklfd, NULL, NULL)) == -1){
            c->message = message_dataconnectionerror;
            send_message(c);
            return 1;
        }

        char message[200];
        sprintf(message, "150 Opening data connection for contents of current directory.\r\n");
        c->message = message;
        send_message(c);

        pthread_t thread_id; 
        pthread_create(&thread_id, NULL, transfer_list, c);

    }else{
        c->message = message_modeundefined;
        send_message(c);
        return 1;
    }

    return 0;
}

int handle_RMD(struct Client* c, char* dir){
    /*
     * Handle RWD command.
     * Return 250 if successful.
     * Retutn 550 if not found.
     */

    if(!c->login){
        c->message = message_unlogin;
        send_message(c);
        return 1;
    }

    char path[200];
    strcpy(path, c->root_dir);
    strcat(path, c->dir);
    strcat(path, dir);

    char message[300];
    if(rmdir(path) == -1){    
        sprintf(message, "550 \"%s\" removed failed.\r\n", dir);
        c->message = message;
        send_message(c);
    }else{
        sprintf(message, "250 \"%s\" removed successfully.\r\n", dir);
        c->message = message;
        send_message(c);
        return 1;
    }


    return 0;
}

int handle_RNFR(struct Client* c, char* dir){
    /*
     * Handle RNFR command.
     * Return 350 if file or directory exists.
     * Return 550 otherwise.
     */

    if(!c->login){
        c->message = message_unlogin;
        send_message(c);
        return 1;
    }

    char path[200];
    strcpy(path, c->root_dir);
    if(dir[0] == '/'){
        strcat(path, dir);
    }else{
        strcat(path, c->dir);
        strcat(path, dir);
    }

    if(access(path, F_OK) != -1){
        strcpy(c->rn_be, path);
        c->message = "350 file exists.\r\n";
        send_message(c);
    }else{
        c->message = "550 file does not exist.\r\n";
        send_message(c);
        return 1;
    }
    return 0;
}

int handle_RNTO(struct Client* c, char* dir){
    /*
     * Handle RNTO command.
     * Return 250 if successful.
     * Return 550 if failed.
     * Return 503 if RNFR not being sent before.
     */

    if(!c->login){
        c->message = message_unlogin;
        send_message(c);
        return 1;
    }

    if(strlen(c->rn_be) == 0){
        c->message = message_rntowithoutrnfr;
        send_message(c);
        return 1;
    }

    char path[200];
    strcpy(path, c->root_dir);
    if(dir[0] == '/'){
        strcat(path, dir);
    }else{
        strcat(path, c->dir);
        strcat(path, dir);
    }

    if(rename(c->rn_be, path) != -1){
        c->message = "250 renamed successfully.\r\n";
        send_message(c);
    }else{
        c->message = "550 renamed failed.\r\n";
        send_message(c);
    }

    memset(c->rn_be, 0, sizeof(c->rn_be));

    return 0;
}
