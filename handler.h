#include "server.h"

int handle_command(struct Client*, char*, char*);

int handle_USER(struct Client*, char*);

int handle_PASS(struct Client*, char*);
