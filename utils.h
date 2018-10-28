#ifndef UTILS_H
#define UTILS_H

#include <time.h>
#include <stdlib.h>
#include "server.h"

int deal_with_parameters(int* port, char* root_dir, int argc, char** argv);

/*
 * send messages to the client according to the integer
 * 0: "220 Anonymous FTP server ready.\r\n"
 */
int send_message(struct Client*);

/* 
 * the number of connections is over the limit.
 */
int over_connections(int fd);

void *transfer_file(void*);

void *store_file(void*);

void *transfer_list(void*);

int get_random_port();

int get_local_ip(int*, int*, int*, int*);

int processing_command(char*, char*, char*);

#endif