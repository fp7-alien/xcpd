#!/bin/sh

service avahi-daemon stop
ip link del V1
ip link del V2
ip link del V3
ip link del V4
