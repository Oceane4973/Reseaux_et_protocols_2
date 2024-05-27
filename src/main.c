#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

    // Affichage des données lues
    for (int i = 0; i < num_routers; i++) {
        printf("Router Name: %s\n", routers[i].name ? routers[i].name : "Unknown");
        for (int j = 0; j < routers[i].num_devices; j++) {
            printf("Device %d: Interface: %s, IP: %s, Mask: %d\n", j + 1,
                   routers[i].devices[j].interface ? routers[i].devices[j].interface : "Unknown",
                   routers[i].devices[j].ip ? routers[i].devices[j].ip : "Unknown",
                   routers[i].devices[j].mask);
        }
    }

    for (int i = 0; i < num_routers; i++) {
        destroyRouter(&routers[i]);
    }

    return EXIT_SUCCESS;
}
