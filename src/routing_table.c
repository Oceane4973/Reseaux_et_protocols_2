#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "routing_table.h"

Routing_table* initRoutingTable(const char *routing_table_path){
    Routing_table *table = malloc(sizeof(Routing_table));
    if (table == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    table->routing_table_path = strdup(routing_table_path);
    if (table->routing_table_path == NULL) {
        perror("strdup");
        free(table);
        exit(EXIT_FAILURE);
    }

    // Initialisez table->table à NULL
    table->table = NULL;
    
    table->num_route = 0;
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