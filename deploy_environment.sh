#!/bin/bash

source scripts/deploy_virtual_ips.sh

ROUTERS_CONFIG_FILE="config/routers_config.yaml"
SERVER_CONFIG_FILE="config/server_config.yaml"

read_router_config() {
    local file=$1

    local router_name=""
    local port=0
    local interface=""
    local interface_ip=""
    local mask=""

    while IFS= read -r line; do
        if [[ $line == *"port:"* ]]; then
            port=$(echo "$line" | awk '{print $2}')
        fi
        if [[ $line == *"name:"* ]]; then
            router_name=$(echo "$line" | awk '{print $3}')
        fi
        if [[ $line == *"ip:"* ]]; then
            interface_ip=$(echo "$line" | awk '{print $2}')
        fi
        if [[ $line == *"mask:"* ]]; then
            mask=$(echo "$line" | awk '{print $2}')
            deploy_device $router_name $interface $interface_ip $mask $port
        fi
        if [[ $line == *"interface:"* ]]; then
            interface=$(echo "$line" | awk '{print $3}')
        fi
    done < "$file"
}

read_server_config() {
    local file=$1

    local server_name=""
    local port=0
    local ip=""
    local mask=""

    while IFS= read -r line; do
        if [[ $line == *"port:"* ]]; then
            port=$(echo "$line" | awk '{print $2}')
        fi
        if [[ $line == *"name:"* ]]; then
            server_name=$(echo "$line" | awk '{print $3}')
        fi
        if [[ $line == *"ip:"* ]]; then
            ip=$(echo "$line" | awk '{print $2}')
        fi
        if [[ $line == *"mask:"* ]]; then
            mask=$(echo "$line" | awk '{print $2}')
            deploy_device $server_name eth0 $ip $mask $port
        fi
    done < "$file"
}

if [ "$EUID" -ne 0 ]; then
  echo "Veuillez exécuter ce script avec des privilèges sudo ou en tant que root."
  exit 1
fi

if [ ! -f "$ROUTERS_CONFIG_FILE" ]; then
    echo "Error: Configuration file '$ROUTERS_CONFIG_FILE' not found."
    exit 1
fi

if [ ! -f "$SERVER_CONFIG_FILE" ]; then
    echo "Error: Configuration file '$SERVER_CONFIG_FILE' not found."
    exit 1
fi

# Lire les configurations des routeurs et les déployer
read_router_config "$ROUTERS_CONFIG_FILE"
read_server_config "$SERVER_CONFIG_FILE"








