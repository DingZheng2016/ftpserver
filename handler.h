#ifndef HANDLER_H
#define HANDLER_H

#include "server.h"

int handle_command(struct Client*, char*, char*);

int handle_USER(struct Client*, char*);

int handle_PASS(struct Client*, char*);

int handle_SYST(struct Client*);

int handle_TYPE(struct Client*, char*);

int handle_QUIT(struct Client*);

int handle_PORT(struct Client*, char*);

int handle_RETR(struct Client*, char*);

int handle_PASV(struct Client*);

int handle_STOR(struct Client*, char*);

int handle_CWD(struct Client*, char*);

int handle_PWD(struct Client*);

int handle_MKD(struct Client*, char*);

int handle_LIST(struct Client*);

int handle_RMD(struct Client*, char*);

int handle_RNFR(struct Client*, char*);

int handle_RNTO(struct Client*, char*);

int handle_REST(struct Client*, char*);

#endif