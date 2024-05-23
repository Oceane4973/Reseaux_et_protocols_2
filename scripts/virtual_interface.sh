#!/bin/bash

delete_device() {
    DEVICE=$1
    ip link delete $DEVICE
}

deploy_device() {
    ROUTER=$1
    INTERFACE=$2
    IP=$3
    MASK=$4
    PORT=$5
    # Afficher les informations du dispositif
    echo "----------------------------------------"
    #echo "Router: $ROUTER"
    #echo "IP: $IP/$MASK :$PORT"
    #echo "Interface: $INTERFACE"
    
    DEVICE="${ROUTER}_${INTERFACE}"
    #delete_device $DEVICE

    ip link add $DEVICE type dummy
    ip link set $DEVICE multicast on
    ip link set $DEVICE arp on
    ip addr add $IP/$MASK brd + dev $DEVICE
    ip link set dev $DEVICE up

    ip addr show $DEVICE
}








