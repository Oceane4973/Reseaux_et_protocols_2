#include <stdio.h>
#include <assert.h>
#include <stdlib.h> // Ajout de l'inclusion de stdlib.h
#include "router.c"
#include "device.c"


void test_calculate_broadcast_address();
void test_device_and_router_constructors();

int main() {
    test_device_and_router_constructors();
    test_calculate_broadcast_address();
    
    return 0;
}

void test_calculate_broadcast_address() {
    const char* ip_address = "192.168.2.5";
    const int cidr = 24;
    const char* expected_broadcast_address = "192.168.2.255"; // Adresse de diffusion attendue
   
    char* broadcast_address = calculate_broadcast_address(ip_address, cidr);
    
    assert(broadcast_address != NULL);
    assert(strcmp(broadcast_address, expected_broadcast_address) == 0);

    printf("Test Passed: calculate_broadcast_address().\n");
    
    free(broadcast_address);
}

void test_device_and_router_constructors() {
    // Devices
    Device device = initDevice("eth0", "127.0.0.1", 24);
    assert(strcmp(device.interface, "eth0") == 0);
    assert(strcmp(device.ip, "127.0.0.1") == 0);
    assert(device.mask == 24);

    printf("Test Passed: initDevice().\n");

    destroyDevice(&device);
    assert(device.interface == NULL);
    assert(device.ip == NULL);

    printf("Test Passed: destroyDevice().\n");
    
    // Router
    Device devices[] = {
        initDevice("eth0", "127.0.0.1", 24),
        initDevice("eth1", "192.1.1.2", 24)
    };
    Router my_router = initRouter("R1", 8000, devices, 2);
    
    assert(strcmp(my_router.name, "R1") == 0);
    assert(my_router.port == 8000);
    assert(my_router.num_devices == 2);

    printf("Test Passed: initRouter().\n");
    
    destroyRouter(&my_router);
    
    assert(my_router.name == NULL);
    assert(my_router.devices == NULL);

    printf("Test Passed: destroyRouter().\n");
}