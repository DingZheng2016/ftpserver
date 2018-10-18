#include <time.h>
#include <stdlib.h>
#include "server.h"

int deal_with_parameters(int* port, char* root_dir, int argc, char** argv);

/*
 * send messages to the client according to the integer
 * 0: "220 Anonymous FTP server ready.\r\n"
 */
int send_message(struct Client*, int);

int get_random_port();

int processing_command(char*, char*, char*);
