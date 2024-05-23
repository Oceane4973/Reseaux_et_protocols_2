#!/bin/bash

source scripts/virtual_interface.sh

CONFIG_FILE="config.yaml"

read_config() {
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

if [ "$EUID" -ne 0 ]; then
  echo "Veuillez exécuter ce script avec des privilèges sudo ou en tant que root."
  exit 1
fi

if [ ! -f "$CONFIG_FILE" ]; then
    echo "Error: Configuration file '$CONFIG_FILE' not found."
    exit 1
fi

# Lire les configurations des routeurs et les déployer
read_config "$CONFIG_FILE"








