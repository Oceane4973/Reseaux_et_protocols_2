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
        table->routing_table_path = strdup(routing_table_path);
    } else {
        table = parse_yaml_file_to_routing_table(file);
        table->routing_table_path = strdup(routing_table_path);
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
                 " { Destination : %s, "
                 " Mask : %d, "
                 " Passerelle : %s, "
                 " Interface : %s, "
                 " Distance : %d },\n", 
                 route->destination ? route->destination : "", 
                 route->mask, 
                 route->passerelle ? route->passerelle : "",
                 route->interface ? route->interface : "",
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

char* routing_table_to_buffer(Routing_table *routing_table) {
    if (!routing_table) {
        fprintf(stderr, "Routing table is NULL\n");
        return NULL;
    }

    char *buffer = (char*)malloc(MAX_BUFFER_SIZE);
    if (!buffer) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    snprintf(buffer, MAX_BUFFER_SIZE, "routes:\n");

    for (int i = 0; i < routing_table->num_route; i++) {
        Route *route = &routing_table->table[i];
        char temp[MAX_CONFIG_LINE_LENGTH]; 

        snprintf(temp, MAX_CONFIG_LINE_LENGTH, "  - destination: %s\n    mask: %d\n    passerelle: %s\n    interface: %s\n    distance: %d\n",
                 route->destination ? route->destination : "",
                 route->mask,
                 route->passerelle ? route->passerelle : "",
                 route->interface ? route->interface : "",
                 route->distance);

        strncat(buffer, temp, MAX_BUFFER_SIZE - strlen(buffer) - 1);
    }

    return buffer;
}

Routing_table* copy_routing_table(const Routing_table *src) {
    Routing_table *dst = (Routing_table *)malloc(sizeof(Routing_table));
    if (!dst) {
        perror("Failed to allocate memory for Routing_table");
        exit(EXIT_FAILURE);
    }
    dst->num_route = src->num_route;
    dst->routing_table_path = safe_strdup(src->routing_table_path);

    dst->table = (Route *)malloc(dst->num_route * sizeof(Route));
    if (!dst->table) {
        perror("Failed to allocate memory for routes");
        free(dst->routing_table_path);
        free(dst);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < dst->num_route; ++i) {
        dst->table[i] = copy_route(&src->table[i]);
    }

    return dst;
}

Route copy_route(const Route *src) {
    Route dst;
    dst.destination = safe_strdup(src->destination);
    dst.mask = src->mask;
    dst.passerelle = safe_strdup(src->passerelle);
    dst.interface = safe_strdup(src->interface);
    dst.distance = src->distance;
    return dst;
}

char* safe_strdup(const char *src) {
    if (!src) return NULL;
    char *dst = strdup(src);
    if (!dst) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    return dst;
}