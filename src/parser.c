#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yaml.h>
#include "parser.h"

Routers* parse_yaml_file_to_router(FILE *file) {
    // Initialisation de l'analyseur YAML
    yaml_parser_t parser;
    yaml_token_t token;
    if (!yaml_parser_initialize(&parser)) {
        perror("Failed to initialize YAML parser");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    yaml_parser_set_input_file(&parser, file);

    // Variables pour stocker les routeurs et les dispositifs
    Router *routers = NULL;
    int num_routers = 0;
    int current_router_index = -1;
    int current_device_index = -1;

    char *current_key = NULL;

    // Parcours des tokens
    while (1) {
        yaml_parser_scan(&parser, &token);
        if (token.type == YAML_STREAM_END_TOKEN) {
            yaml_token_delete(&token);
            break;
        }

        switch (token.type) {
            case YAML_KEY_TOKEN:
                // Attendre la prochaine valeur
                yaml_token_delete(&token);
                yaml_parser_scan(&parser, &token);
                current_key = strdup((char *)token.data.scalar.value);
                yaml_token_delete(&token);
                break;

            case YAML_VALUE_TOKEN:
                // Attendre la prochaine valeur
                yaml_token_delete(&token);
                yaml_parser_scan(&parser, &token);

                if (current_router_index == -1) {
                    // Nouveau routeur
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
                } else if (current_key && strcmp(current_key, "port") == 0 && current_device_index == -1) {
                    routers[current_router_index].port = atoi((char*)token.data.scalar.value);
                } else if (current_key && (strcmp(current_key, "interface") == 0 || strcmp(current_key, "ip") == 0 || strcmp(current_key, "mask") == 0)) {
                    // Nouveau dispositif
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

                    // Traitement des clés du dispositif
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
                    // Fin du mappage pour le dispositif
                    current_device_index = -1;
                } else if (current_router_index != -1) {
                    // Fin du mappage pour le routeur
                    current_router_index = -1;
                    // Réinitialiser l'index du dispositif après avoir ajouté tous les dispositifs au routeur
                    current_device_index = -1;
                }
                yaml_token_delete(&token);
                break;

            default:
                yaml_token_delete(&token);
                break;
        }
    }

    // Libération des ressources utilisées par l'analyseur YAML
    yaml_parser_delete(&parser);
    fclose(file);

    Routers *routers_tab = (Routers *)malloc(sizeof(Routers));
    routers_tab->num_routers = num_routers;
    routers_tab->routers = routers;

    return routers_tab;
}