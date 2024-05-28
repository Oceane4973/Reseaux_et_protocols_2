#include "device.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Device initDevice(const char *interface, const char *ip, int mask) {
    Device device;
    device.interface = strdup(interface);
    if (!device.interface) {
        perror("Failed to allocate memory for device interface");
        exit(EXIT_FAILURE);
    }

    device.ip = strdup(ip);
    if (!device.ip) {
        perror("Failed to allocate memory for device IP");
        exit(EXIT_FAILURE);
    }

    device.mask = mask;
    return device;
}

void destroyDevice(Device *device) {
    free(device->interface);
    free(device->ip);
    device->interface = NULL;
    device->ip = NULL;
}