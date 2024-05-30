#ifndef ROUTING_TABLE_H
#define ROUTING_TABLE_H

#include <stdio.h>

typedef struct {
    char *destination;
    int mask;
    char *passerelle;
    char *interface;
    int distance;
} Route;

typedef struct {
    Route* table;
    char* routing_table_path;
    int num_route;
} Routing_table;

Routing_table* initRoutingTable(const char *routing_table_path);
void destroyRoutingTable(Routing_table* routing_table);
void destroyRoute(Route* route);
char* displayRoutingTable(Routing_table *routing_table);
char* routing_table_to_buffer(Routing_table *routing_table);
Routing_table* copy_routing_table(const Routing_table *src);
Route copy_route(const Route *src);

char* safe_strdup(const char *src);
#endif
