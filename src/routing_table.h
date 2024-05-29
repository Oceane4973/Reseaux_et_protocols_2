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

#endif
