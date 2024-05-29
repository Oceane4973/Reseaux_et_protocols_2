#include <stdio.h>
#include <assert.h>
#include <stdlib.h> // Ajout de l'inclusion de stdlib.h
#include "router.h"
#include "parser.h"

void test_calculate_broadcast_address();
void test_yaml_file_parser_to_router();
void test_device_and_router_constructors();
void test_yaml_file_parser_to_routing_table();
void test_updateRoutingTable();

int main() {
    test_device_and_router_constructors();
    test_calculate_broadcast_address();
    test_yaml_file_parser_to_router();
    test_yaml_file_parser_to_routing_table();
    test_updateRoutingTable();
    
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

void test_yaml_file_parser_to_router(){
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

    Routers* routers_list = parse_yaml_file_to_router(file);
    const int num_routers = routers_list->num_routers;
    Router* routers = routers_list->routers;

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

    printf("Test Passed: yaml_file_parser_to_router().\n");
    
    for (int i = 0; i < num_routers; i++) {
        destroyRouter(&routers[i]);
    }
}

void test_yaml_file_parser_to_routing_table(){
    const char *yaml_content = 
    "routes:\n"
    "  - destination: 127.0.0.0\n"
    "    mask: 24\n"
    "    passerelle:\n"
    "    interface: 127.0.0.1\n"
    "    distance: 1\n"
    "  - destination: 10.1.1.0\n"
    "    mask: 30\n"
    "    passerelle:\n"
    "    interface: 10.1.1.2\n"
    "    distance: 1\n";

    FILE *file = fmemopen((void *)yaml_content, strlen(yaml_content), "r");

    Routing_table* routing_table = parse_yaml_file_to_routing_table(file);
    const int num_routes = routing_table->num_route;
    Route* tables = routing_table->table;
    
    assert(num_routes == 2);

    assert(strcmp(tables[0].destination, "127.0.0.0") == 0);
    assert(tables[0].mask == 24);
    assert(strcmp(tables[0].passerelle, "") == 0);
    assert(strcmp(tables[0].interface, "127.0.0.1") == 0);
    assert(tables[0].distance == 1);

    assert(strcmp(tables[1].destination, "10.1.1.0") == 0);
    assert(tables[1].mask == 30);
    assert(strcmp(tables[1].passerelle, "") == 0);
    assert(strcmp(tables[1].interface, "10.1.1.2") == 0);
    assert(tables[1].distance == 1);

    printf("Test Passed: parse_yaml_file_to_routing_table().\n");

    destroyRoutingTable(routing_table);
}

void test_updateRoutingTable(){
    //création d'un routeur
    Device devices[] = {
        initDevice("eth0", "127.0.0.1", 24),
        initDevice("eth1", "192.1.1.2", 24)
    };
    Router my_router = initRouter("R1", 8000, devices, 2);

    //creation d'une table de routage
    const char *yaml_content = 
    "routes:\n"
    "  - destination: 127.0.0.0\n"
    "    mask: 24\n"
    "    passerelle:\n"
    "    interface: 127.0.0.1\n"
    "    distance: 1\n"
    "  - destination: 10.1.2.0\n"
    "    mask: 30\n"
    "    passerelle: 10.1.1.1\n"
    "    interface: 10.1.1.2\n"
    "    distance: 3\n"
    "  - destination: 10.1.2.0\n"
    "    mask: 30\n"
    "    passerelle: 10.1.1.3\n"
    "    interface: 10.1.1.2\n"
    "    distance: 2\n";

    FILE *file = fmemopen((void *)yaml_content, strlen(yaml_content), "r");
    Routing_table* routing_table = parse_yaml_file_to_routing_table(file);


    assert(my_router.routing_table->num_route == 2);
    assert(routing_table->num_route == 3);

    updateRoutingTable(&my_router, routing_table);

    //char *routing_table_str = displayRoutingTable(my_router.routing_table);
    //printf("%s", routing_table_str);

    assert(my_router.routing_table->num_route == 3);

    assert(strcmp(my_router.routing_table->table[0].destination, "127.0.0.0") == 0);
    assert(strcmp(my_router.routing_table->table[0].passerelle, "") == 0);
    assert(strcmp(my_router.routing_table->table[0].interface, "127.0.0.1") == 0);

    assert(strcmp(my_router.routing_table->table[2].destination, "10.1.2.0") == 0);
    assert(strcmp(my_router.routing_table->table[2].passerelle, "10.1.1.3") == 0);
    assert(strcmp(my_router.routing_table->table[2].interface, "10.1.1.2") == 0);
    assert(my_router.routing_table->table[2].distance == 2);
    
    printf("Test Passed: update_routing_table().\n");

    //free(routing_table_str);
    destroyRoutingTable(routing_table);
    destroyRouter(&my_router);
}