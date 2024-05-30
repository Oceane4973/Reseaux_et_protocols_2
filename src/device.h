#ifndef DEVICE_H
#define DEVICE_H

#include "router.h"

#define INTERVAL 1

typedef struct {
    char *interface;
    char *ip;
    int mask;
} Device;


Device initDevice(const char *interface, const char *ip, int mask);
void destroyDevice(Device *device);
void *deviceThread(void *ThreadDevicesArg);
void *send_on_broadcast(void *threadBroadcastArg);

#endif