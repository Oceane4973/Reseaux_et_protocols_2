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

void displayRoutingTable(Routing_table *routing_table) {
    if (!routing_table) {
        printf("Routing table is NULL.\n");
        return;
    }

    printf("Routing Table Path: %s\n", routing_table->routing_table_path ? routing_table->routing_table_path : "No path specified");
    printf("Number of Routes: %d\n", routing_table->num_route);
    printf("Routes:\n");

    for (int i = 0; i < routing_table->num_route; i++) {
        Route *route = &routing_table->table[i];
        printf("  Route %d:\n", i + 1);
        printf("    Destination: %s\n", route->destination ? route->destination : "N/A");
        printf("    Mask: %d\n", route->mask);
        printf("    Passerelle: %s\n", route->passerelle ? route->passerelle : "N/A");
        printf("    Interface: %s\n", route->interface ? route->interface : "N/A");
        printf("    Distance: %d\n", route->distance);
    }
}