#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "parser.h"
#include "router.h"

int main() {
    // Ouverture du fichier YAML
    FILE *file = fopen("config.yaml", "r");
    if (!file) {
        perror("Failed to open file");
        return EXIT_FAILURE;
    }

    Router* routers = parse_yaml_file(file);
    const int num_routers = routers->num_devices;

    pthread_t threads[num_routers];
    ThreadRouterArg threadData[num_routers];
    
    for (int i = 0; i < num_routers; i++) {
        threadData[i].router = routers[i];
        pthread_create(&threads[i], NULL, startRouter, (void *)&threadData[i]);
    }
    
    for (int i = 0; i < num_routers; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < num_routers; i++) {
        destroyRouter(&routers[i]);
    }

    return EXIT_SUCCESS;
}
