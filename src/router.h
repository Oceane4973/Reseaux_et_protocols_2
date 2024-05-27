#ifndef ROUTER_H
#define ROUTER_H

#include "device.h"

#define MAX_CONFIG_LINE_LENGTH 256
#define MAX_BUFFER_SIZE 1024

#define BROADCAST_PORT 1900

// Structure représentant le routeur
typedef struct {
    char *name;
    int port;
    Device *devices;
    int num_devices;
} Router;

// Déclaration des fonctions
Router initRouter(const char *name, int port, Device *devices, int num_devices);
void destroyRouter(Router *router);
void startRouter(Router router);

char* calculate_broadcast_address(const char* ip_address, const int cidr);

#endif