#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "parser.h"
#include "router.h"
#include <stdbool.h>
#include "server.h"

int main() {
    FILE *routers_file = fopen("config/routers_config.yaml", "r");
    if (!routers_file) {
        perror("Failed to open routers_file");
        return EXIT_FAILURE;
    }

    FILE *server_file = fopen("config/server_config.yaml", "r");
    if (!server_file) {
        perror("fmemopen failed");
        exit(EXIT_FAILURE);
    }

    Server *server = parse_yaml_file_to_server(server_file);
    Routers* routers = parse_yaml_file_to_router(routers_file);
    const int num_routers = routers->num_routers;

    pthread_t threads[num_routers+1];
    ThreadRouterArg threadData[num_routers+1];
    
    for (int i = 0; i < num_routers; i++) {
        threadData[i].router = routers->routers[i];
        pthread_create(&threads[i], NULL, startRouter, (void *)&threadData[i]);
    }

    pthread_create(&threads[num_routers], NULL, start_server, (void *)server);
    
    for (int i = 0; i < num_routers; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < num_routers; i++) {
        destroyRouter(&routers->routers[i]);
    }

    destroyServer(server);

    return EXIT_SUCCESS;
}
