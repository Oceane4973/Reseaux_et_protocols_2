#ifndef DEVICE_H
#define DEVICE_H

typedef struct {
    char *interface;
    char *ip;
    int mask;
} Device;

Device initDevice(const char *interface, const char *ip, int mask);
void destroyDevice(Device *device);

#endif