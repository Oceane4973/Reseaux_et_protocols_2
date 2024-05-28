#include <stdio.h>
#include <assert.h>
#include <stdlib.h> // Ajout de l'inclusion de stdlib.h
#include "router.h"
#include "parser.h"

void test_calculate_broadcast_address();
void test_yaml_file_parser();
void test_device_and_router_constructors();

int main() {
    test_device_and_router_constructors();
    test_calculate_broadcast_address();
    test_yaml_file_parser();
    
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
    assert(strcmp(my_router.routing_table->routing_table_path, "config/R1/routing_table.yaml")==0);

    printf("Test Passed: initRouter().\n");
    
    destroyRouter(&my_router);
    
    assert(my_router.name == NULL);
    assert(my_router.devices == NULL);

    printf("Test Passed: destroyRouter().\n");
}

void test_yaml_file_parser(){
    const char *yaml_content = 
    "routers:\n"
    "  - name: R1\n"
    "    port: 8520\n"
    "    devices:\n"
    "      - interface: eth0\n"
    "        ip: 127.0.0.1\n"
    "        mask: 24\n"
    "      - interface: eth1\n"
    "        ip: 192.1.1.2\n"
    "        mask: 24\n"
    "  - name: R2\n"
    "    port: 8520\n"
    "    devices:\n"
    "      - interface: eth0\n"
    "        ip: 192.1.1.3\n"
    "        mask: 24\n";

    FILE *file = fmemopen((void *)yaml_content, strlen(yaml_content), "r");

    Router* routers = parse_yaml_file_to_router(file);
    const int num_routers = routers->num_devices;

    assert(num_routers == 2);

    assert(strcmp(routers[0].name, "R1") == 0);
    assert(routers[0].port == 8520);

    assert(strcmp(routers[1].name, "R2") == 0);
    assert(routers[1].port == 8520);

    assert(routers[0].num_devices == 2);
    assert(routers[1].num_devices == 1);

    assert(strcmp(routers[0].devices[0].interface,"eth0")==0);
    assert(strcmp(routers[0].devices[0].ip,"127.0.0.1")==0);
    assert(routers[0].devices[0].mask == 24);

    assert(strcmp(routers[0].devices[1].interface,"eth1")==0);
    assert(strcmp(routers[0].devices[1].ip,"192.1.1.2")==0);
    assert(routers[0].devices[1].mask == 24);

    assert(strcmp(routers[1].devices[0].interface,"eth0")==0);
    assert(strcmp(routers[1].devices[0].ip,"192.1.1.3")==0);
    assert(routers[1].devices[0].mask == 24);

    printf("Test Passed: parse_yaml_file().\n");
    
    for (int i = 0; i < num_routers; i++) {
        destroyRouter(&routers[i]);
    }
}