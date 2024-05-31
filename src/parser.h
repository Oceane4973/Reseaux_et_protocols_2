#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "router.h"
#include "server.h"

Routers* parse_yaml_file_to_router(FILE *file);
Server* parse_yaml_file_to_server(FILE *file);
Routing_table* parse_yaml_file_to_routing_table(FILE *file);