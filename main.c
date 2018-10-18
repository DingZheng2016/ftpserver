#include <stdio.h>
#include "server.h"
#include "utils.h"

int main(int argc, char** argv){
    int port = 21;
    char root_dir[100] = "/tmp";

    if(deal_with_parameters(&port, root_dir, argc, argv) == -1){
        printf("Parameters Error.");
        return 0;
    }

    struct Server *sv = (struct Server*)malloc(sizeof(struct Server));

    init_server(sv, port, root_dir);
    run_server(sv);

    return 0;
}
