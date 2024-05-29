#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "routing_table.h"
#include "parser.h"

Routing_table* initRoutingTable(const char *routing_table_path) {
    FILE *file = fopen(routing_table_path, "r");
    Routing_table *table;

    if (!file) {
        table = (Routing_table *)malloc(sizeof(Routing_table));
        if (!table) {
            perror("Failed to allocate memory for routing_table");
            exit(EXIT_FAILURE);
        }
        table->table = NULL;
        table->num_route = 0;
        table->routing_table_path = strdup(routing_table_path); // Déplacer l'affectation ici
    } else {
        table = parse_yaml_file_to_routing_table(file);
        table->routing_table_path = strdup(routing_table_path); // Déplacer l'affectation ici
        fclose(file); 
    }

    return table;
}
void destroyRoutingTable(Routing_table* routing_table) {
    if (routing_table == NULL) {
        return;
    }

    if (routing_table->table != NULL) {
        for (int i = 0; i < routing_table->num_route; ++i) {
            destroyRoute(&(routing_table->table[i]));
        }

        free(routing_table->table);
    }

    free(routing_table->routing_table_path);
    free(routing_table);
}

void destroyRoute(Route* route) {
    if (route == NULL) {
        return;
    }

    free(route->destination);
    free(route->passerelle);
    free(route->interface);
}

char* displayRoutingTable(Routing_table *routing_table) {
    if (!routing_table) {
        return strdup("Routing table is NULL.\n");
    }

    size_t buffer_size = 1024;
    char *result = (char *)malloc(buffer_size);
    if (!result) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    result[0] = '\0';

    for (int i = 0; i < routing_table->num_route; i++) {
        Route *route = &routing_table->table[i];
        
        char route_info[256];
        snprintf(route_info, sizeof(route_info),
                 //" { Route : %d, "
                 " { Destination : %s, "
                 " Mask : %d, "
                 " Passerelle : %s, "
                 " Interface : %s, "
                 " Distance : %d },\n", 
                 //i + 1, 
                 route->destination ? route->destination : "N/A", 
                 route->mask, 
                 route->passerelle ? route->passerelle : "N/A",
                 route->interface ? route->interface : "N/A",
                 route->distance);

        if (strlen(result) + strlen(route_info) + 1 > buffer_size) {
            buffer_size *= 2;
            result = (char *)realloc(result, buffer_size);
            if (!result) {
                perror("Failed to reallocate memory");
                exit(EXIT_FAILURE);
            }
        }
        strcat(result, route_info);
    }

    return result;
}
