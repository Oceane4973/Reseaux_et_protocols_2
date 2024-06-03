#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yaml.h>
#include "parser.h"

Routers* parse_yaml_file_to_router(FILE *file) {
    yaml_parser_t parser;
    yaml_token_t token;
    if (!yaml_parser_initialize(&parser)) {
        perror("Failed to initialize YAML parser");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    yaml_parser_set_input_file(&parser, file);

    Router *routers = NULL;
    int num_routers = 0;
    int current_router_index = -1;
    int current_device_index = -1;

    char *current_key = NULL;

    while (1) {
        yaml_parser_scan(&parser, &token);
        if (token.type == YAML_STREAM_END_TOKEN) {
            yaml_token_delete(&token);
            break;
        }

        switch (token.type) {
            case YAML_KEY_TOKEN:
                yaml_token_delete(&token);
                yaml_parser_scan(&parser, &token);
                current_key = strdup((char *)token.data.scalar.value);
                yaml_token_delete(&token);
                break;

            case YAML_VALUE_TOKEN:
                yaml_token_delete(&token);
                yaml_parser_scan(&parser, &token);

                if (current_router_index == -1) {
                    routers = realloc(routers, (num_routers + 1) * sizeof(Router));
                    if (!routers) {
                        perror("Failed to allocate memory for routers");
                        yaml_token_delete(&token);
                        yaml_parser_delete(&parser);
                        fclose(file);
                        exit(EXIT_FAILURE);
                    }

                    current_router_index = num_routers;
                    routers[current_router_index].devices = NULL;
                    routers[current_router_index].num_devices = 0;
                    routers[current_router_index].name = NULL;
                    num_routers++;
                }

                if (current_key && strcmp(current_key, "name") == 0 && current_device_index == -1) {
                    routers[current_router_index].name = strdup((char *)token.data.scalar.value);
                    char routing_table_path[MAX_PATH_LENGTH];
                    sprintf(routing_table_path, "%s%s/routing_table.yaml", GLOBAL_ROUTING_TABLE_PATH, routers[current_router_index].name);
                    routers[current_router_index].routing_table = initRoutingTable(routing_table_path);
                } else if (current_key && strcmp(current_key, "port") == 0 && current_device_index == -1) {
                    routers[current_router_index].port = atoi((char*)token.data.scalar.value);
                } else if (current_key && (strcmp(current_key, "interface") == 0 || strcmp(current_key, "ip") == 0 || strcmp(current_key, "mask") == 0)) {
                    Router *current_router = &routers[current_router_index];
                    if (current_device_index == -1 || current_device_index == current_router->num_devices) {
                        current_router->devices = realloc(current_router->devices, (current_router->num_devices + 1) * sizeof(Device));
                        if (!current_router->devices) {
                            perror("Failed to allocate memory for devices");
                            yaml_token_delete(&token);
                            yaml_parser_delete(&parser);
                            fclose(file);
                            exit(EXIT_FAILURE);
                        }
                        current_device_index = current_router->num_devices;
                        current_router->devices[current_device_index].interface = NULL;
                        current_router->devices[current_device_index].ip = NULL;
                        current_router->devices[current_device_index].mask = 0;
                        current_router->num_devices++;
                    }

                    Device *current_device = &current_router->devices[current_device_index];
                    if (strcmp(current_key, "interface") == 0) {
                        current_device->interface = strdup((char *)token.data.scalar.value);
                    } else if (strcmp(current_key, "ip") == 0) {
                        current_device->ip = strdup((char *)token.data.scalar.value);
                    } else if (strcmp(current_key, "mask") == 0) {
                        current_device->mask = atoi((char *)token.data.scalar.value);
                    }
                }

                free(current_key);
                current_key = NULL;
                yaml_token_delete(&token);
                break;

            case YAML_BLOCK_MAPPING_START_TOKEN:
                yaml_token_delete(&token);
                break;

            case YAML_BLOCK_END_TOKEN:
                if (current_device_index != -1) {
                    current_device_index = -1;
                } else if (current_router_index != -1) {
                    current_router_index = -1;
                    current_device_index = -1;
                }
                yaml_token_delete(&token);
                break;

            default:
                yaml_token_delete(&token);
                break;
        }
    }

    yaml_parser_delete(&parser);
    fclose(file);

    Routers *routers_tab = (Routers *)malloc(sizeof(Routers));
    routers_tab->num_routers = num_routers;
    routers_tab->routers = routers;

    return routers_tab;
}

Routing_table* parse_yaml_file_to_routing_table(FILE *file) {
    yaml_parser_t parser;
    yaml_document_t document;
    Routing_table *routing_table = NULL;

    if (!yaml_parser_initialize(&parser)) {
        fprintf(stderr, "Failed to initialize parser!\n");
        return NULL;
    }

    yaml_parser_set_input_file(&parser, file);

    if (!yaml_parser_load(&parser, &document)) {
        fprintf(stderr, "Failed to load YAML document!\n");
        yaml_parser_delete(&parser);
        return NULL;
    }

    yaml_node_t *root = yaml_document_get_root_node(&document);
    if (!root || root->type != YAML_MAPPING_NODE) {
        fprintf(stderr, "Invalid YAML format!\n");
        yaml_document_delete(&document);
        yaml_parser_delete(&parser);
        return NULL;
    }

    routing_table = (Routing_table*)malloc(sizeof(Routing_table));
    if (!routing_table) {
        perror("Failed to allocate memory for routing_table");
        yaml_document_delete(&document);
        yaml_parser_delete(&parser);
        return NULL;
    }

    routing_table->table = NULL;
    routing_table->num_route = 0;
    routing_table->routing_table_path = NULL;

    for (yaml_node_pair_t *pair = root->data.mapping.pairs.start;
         pair < root->data.mapping.pairs.top; pair++) {
        
        yaml_node_t *key = yaml_document_get_node(&document, pair->key);
        yaml_node_t *value = yaml_document_get_node(&document, pair->value);

        if (strcmp((char *)key->data.scalar.value, "routes") == 0 && value->type == YAML_SEQUENCE_NODE) {
            routing_table->num_route = value->data.sequence.items.top - value->data.sequence.items.start;
            routing_table->table = (Route*)malloc(routing_table->num_route * sizeof(Route));
            if (!routing_table->table) {
                perror("Failed to allocate memory for routes table");
                free(routing_table);
                yaml_document_delete(&document);
                yaml_parser_delete(&parser);
                return NULL;
            }

            int route_index = 0;
            for (yaml_node_item_t *item = value->data.sequence.items.start;
                 item < value->data.sequence.items.top; item++) {
                yaml_node_t *route_node = yaml_document_get_node(&document, *item);

                if (route_node->type != YAML_MAPPING_NODE) {
                    continue;
                }

                Route *route = &routing_table->table[route_index++];
                memset(route, 0, sizeof(Route));

                for (yaml_node_pair_t *route_pair = route_node->data.mapping.pairs.start;
                     route_pair < route_node->data.mapping.pairs.top; route_pair++) {
                    
                    yaml_node_t *route_key = yaml_document_get_node(&document, route_pair->key);
                    yaml_node_t *route_value = yaml_document_get_node(&document, route_pair->value);

                    if (strcmp((char *)route_key->data.scalar.value, "destination") == 0) {
                        route->destination = strdup((char *)route_value->data.scalar.value);
                    } else if (strcmp((char *)route_key->data.scalar.value, "mask") == 0) {
                        route->mask = atoi((char *)route_value->data.scalar.value);
                    } else if (strcmp((char *)route_key->data.scalar.value, "passerelle") == 0) {
                        route->passerelle = route_value->data.scalar.value ? strdup((char *)route_value->data.scalar.value) : NULL;
                    } else if (strcmp((char *)route_key->data.scalar.value, "interface") == 0) {
                        route->interface = strdup((char *)route_value->data.scalar.value);
                    } else if (strcmp((char *)route_key->data.scalar.value, "distance") == 0) {
                        route->distance = atoi((char *)route_value->data.scalar.value);
                    }
                }
            }
        }
    }

    yaml_document_delete(&document);
    yaml_parser_delete(&parser);
    return routing_table;
}

Routing_table* parse_yaml_buffer_to_routing_table(const char *buffer, size_t size){
    yaml_parser_t parser;
    yaml_document_t document;
    Routing_table *routing_table = NULL;

    if (!yaml_parser_initialize(&parser)) {
        fprintf(stderr, "Failed to initialize parser!\n");
        return NULL;
    }

    yaml_parser_set_input_string(&parser, (const unsigned char *)buffer, size);

    if (!yaml_parser_load(&parser, &document)) {
        fprintf(stderr, "Failed to load YAML document!\n");
        yaml_parser_delete(&parser);
        return NULL;
    }

    yaml_node_t *root = yaml_document_get_root_node(&document);
    if (!root || root->type != YAML_MAPPING_NODE) {
        fprintf(stderr, "Invalid YAML format!\n");
        yaml_document_delete(&document);
        yaml_parser_delete(&parser);
        return NULL;
    }

    routing_table = (Routing_table*)malloc(sizeof(Routing_table));
    if (!routing_table) {
        perror("Failed to allocate memory for routing_table");
        yaml_document_delete(&document);
        yaml_parser_delete(&parser);
        return NULL;
    }

    routing_table->table = NULL;
    routing_table->num_route = 0;
    routing_table->routing_table_path = NULL;

    for (yaml_node_pair_t *pair = root->data.mapping.pairs.start;
         pair < root->data.mapping.pairs.top; pair++) {
        
        yaml_node_t *key = yaml_document_get_node(&document, pair->key);
        yaml_node_t *value = yaml_document_get_node(&document, pair->value);

        if (strcmp((char *)key->data.scalar.value, "routes") == 0 && value->type == YAML_SEQUENCE_NODE) {
            routing_table->num_route = value->data.sequence.items.top - value->data.sequence.items.start;
            routing_table->table = (Route*)malloc(routing_table->num_route * sizeof(Route));
            if (!routing_table->table) {
                perror("Failed to allocate memory for routes table");
                free(routing_table);
                yaml_document_delete(&document);
                yaml_parser_delete(&parser);
                return NULL;
            }

            int route_index = 0;
            for (yaml_node_item_t *item = value->data.sequence.items.start;
                 item < value->data.sequence.items.top; item++) {
                yaml_node_t *route_node = yaml_document_get_node(&document, *item);

                if (route_node->type != YAML_MAPPING_NODE) {
                    continue;
                }

                Route *route = &routing_table->table[route_index++];
                memset(route, 0, sizeof(Route));

                for (yaml_node_pair_t *route_pair = route_node->data.mapping.pairs.start;
                     route_pair < route_node->data.mapping.pairs.top; route_pair++) {
                    
                    yaml_node_t *route_key = yaml_document_get_node(&document, route_pair->key);
                    yaml_node_t *route_value = yaml_document_get_node(&document, route_pair->value);

                    if (strcmp((char *)route_key->data.scalar.value, "destination") == 0) {
                        route->destination = strdup((char *)route_value->data.scalar.value);
                    } else if (strcmp((char *)route_key->data.scalar.value, "mask") == 0) {
                        route->mask = atoi((char *)route_value->data.scalar.value);
                    } else if (strcmp((char *)route_key->data.scalar.value, "passerelle") == 0) {
                        route->passerelle = route_value->data.scalar.value ? strdup((char *)route_value->data.scalar.value) : NULL;
                    } else if (strcmp((char *)route_key->data.scalar.value, "interface") == 0) {
                        route->interface = strdup((char *)route_value->data.scalar.value);
                    } else if (strcmp((char *)route_key->data.scalar.value, "distance") == 0) {
                        route->distance = atoi((char *)route_value->data.scalar.value);
                    }
                }
            }
        }
    }

    yaml_document_delete(&document);
    yaml_parser_delete(&parser);
    return routing_table;
}

Server* parse_yaml_file_to_server(FILE *file) {
    yaml_parser_t parser;
    yaml_token_t token;

    if (!yaml_parser_initialize(&parser)) {
        perror("Failed to initialize YAML parser");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    yaml_parser_set_input_file(&parser, file);

    Server *server = (Server*)malloc(sizeof(Server));
    if (!server) {
        perror("Failed to allocate memory for server");
        yaml_parser_delete(&parser);
        fclose(file);
        exit(EXIT_FAILURE);
    }
    server->name = NULL;
    server->ip = NULL;
    server->port = 0;
    server->mask = 0;

    char *current_key = NULL;

    while (1) {
        yaml_parser_scan(&parser, &token);
        if (token.type == YAML_STREAM_END_TOKEN) {
            yaml_token_delete(&token);
            break;
        }

        switch (token.type) {
            case YAML_KEY_TOKEN:
                yaml_token_delete(&token);
                yaml_parser_scan(&parser, &token);
                current_key = strdup((char *)token.data.scalar.value);
                yaml_token_delete(&token);
                break;

            case YAML_VALUE_TOKEN:
                yaml_token_delete(&token);
                yaml_parser_scan(&parser, &token);

                if (current_key && strcmp(current_key, "name") == 0) {
                    server->name = strdup((char *)token.data.scalar.value);
                } else if (current_key && strcmp(current_key, "port") == 0) {
                    server->port = atoi((char *)token.data.scalar.value);
                } else if (current_key && strcmp(current_key, "ip") == 0) {
                    server->ip = strdup((char *)token.data.scalar.value);
                } else if (current_key && strcmp(current_key, "mask") == 0) {
                    server->mask = atoi((char *)token.data.scalar.value);
                }

                free(current_key);
                current_key = NULL;
                yaml_token_delete(&token);
                break;

            case YAML_BLOCK_MAPPING_START_TOKEN:
            case YAML_BLOCK_END_TOKEN:
            case YAML_STREAM_START_TOKEN:
            case YAML_STREAM_END_TOKEN:
            case YAML_NO_TOKEN:
            case YAML_VERSION_DIRECTIVE_TOKEN:
            case YAML_TAG_DIRECTIVE_TOKEN:
            case YAML_DOCUMENT_START_TOKEN:
            case YAML_DOCUMENT_END_TOKEN:
            case YAML_BLOCK_SEQUENCE_START_TOKEN:
            case YAML_BLOCK_ENTRY_TOKEN:
            case YAML_FLOW_SEQUENCE_START_TOKEN:
            case YAML_FLOW_SEQUENCE_END_TOKEN:
            case YAML_FLOW_MAPPING_START_TOKEN:
            case YAML_FLOW_MAPPING_END_TOKEN:
            case YAML_FLOW_ENTRY_TOKEN:
            case YAML_ALIAS_TOKEN:
            case YAML_ANCHOR_TOKEN:
            case YAML_TAG_TOKEN:
            case YAML_SCALAR_TOKEN:
            default:
                yaml_token_delete(&token);
                break;
        }
    }

    yaml_parser_delete(&parser);
    fclose(file);

    return server;
}