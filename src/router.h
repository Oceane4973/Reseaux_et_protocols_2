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

// Structure représentant le routeur
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

// Déclaration des fonctions
Router initRouter(const char *name, int port, Device *devices, int num_devices);
void destroyRouter(Router *router);
void *deviceThread(void *ThreadDevicesArg);
void *startRouter(void *ThreadRouterArg);

char* calculate_broadcast_address(const char* ip_address, const int cidr);

#endif