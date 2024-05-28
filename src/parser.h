#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "router.h"

Router* parse_yaml_file_to_router(FILE *file);
Routing_table* parse_yaml_file_to_routing_table(FILE *file);