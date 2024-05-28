#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "router.h"

typedef struct {
    Router *routers;
    int num_routers;
} Routers;

Routers* parse_yaml_file_to_router(FILE *file);
Routing_table* parse_yaml_file_to_routing_table(FILE *file);