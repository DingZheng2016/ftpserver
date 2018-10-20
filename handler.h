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
