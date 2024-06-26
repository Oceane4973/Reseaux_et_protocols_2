#ifndef ROUTER_H
#define ROUTER_H

#include "device.h"
#include "routing_table.h"

#define GLOBAL_ROUTING_TABLE_PATH "config/"

#define MAX_CONFIG_LINE_LENGTH 256
#define MAX_BUFFER_SIZE 1024
#define BUFFER_SIZE 1024
#define BROADCAST_PORT 1900
#define MAX_PATH_LENGTH 100

typedef struct {
    char *name;
    int port;
    Device *devices;
    int num_devices;
    Routing_table* routing_table;
} Router;

typedef struct {
    Device *device;
    Router* router;
    int port;
} ThreadDevicesArg;

typedef struct {
    Router router;
} ThreadRouterArg ;

typedef struct {
    Router *routers;
    int num_routers;
} Routers;

typedef struct {
    char* broadcast_adrr; 
    int port;
    Device* device;
    Router * router;
} ThreadBroadcastArg;

Router initRouter(const char *name, int port, Device *devices, int num_devices);
void destroyRouter(Router *router);
void *startRouter(void *ThreadRouterArg);
void updateRoutingTable(Router *router, Routing_table *routing_table);

char* calculate_broadcast_address(const char* ip_address, const int cidr);
void fill_empty_gateways(Routing_table *routing_table, const char *default_gateway);
char* calculate_network_address(const char *ip_str, int cidr_mask);

#endif