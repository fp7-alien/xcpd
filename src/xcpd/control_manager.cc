/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "control_manager.h"

/**
* @file control_manager.cc
* @author Richard Clegg richard@richardclegg.org
*
* @brief Manages control path daemon parameters
*/

using namespace rofl;
using namespace xdpd;

control_manager* control_manager::cm_instance= NULL;


control_manager* control_manager::Instance()
{
    if (!cm_instance) {
        cm_instance= new control_manager;
    }
    return cm_instance;
}

void control_manager::init()
// Set default 
{
    switch_ip= "127.0.0.1";
    switch_port= 6631;
    xcpd_ip="127.0.0.1";
    xcpd_port=6632;
    higher_ip="127.0.0.1";
    higher_port=6633;
    switch_addr= caddress(AF_INET, switch_ip.c_str(),switch_port);
    xcpd_addr= caddress(AF_INET, xcpd_ip.c_str(),xcpd_port);
    higher_addr= caddress(AF_INET, higher_ip.c_str(),higher_port);
    switch_name= std::string();
    port_names= std::vector<std::string>();
    switch_to_xcpd_conn= ACTIVE_CONNECTION;
    xcpd_to_control_conn= ACTIVE_CONNECTION;
}

void control_manager::set_higher_address(caddress &c)
{
    higher_addr=c;
}

caddress control_manager::get_higher_address()
{
    return higher_addr;
}


void control_manager::set_xcpd_address(caddress &c)
{
    xcpd_addr=c;
}

caddress control_manager::get_xcpd_address()
{
    return xcpd_addr;
}


void control_manager::set_switch_address(caddress &c)
{
    switch_addr=c;
}

caddress control_manager::get_switch_address()
{
    return switch_addr;
}

void control_manager::set_switch_ip(std::string ip)
{
    switch_ip= ip;
    switch_addr= caddress(AF_INET, switch_ip.c_str(),switch_port);
}
std::string control_manager::get_switch_ip()
{
    return switch_ip;
}

void control_manager::set_switch_port(int p)
{
    switch_port=p;
    switch_addr= caddress(AF_INET, switch_ip.c_str(),switch_port);
}

int control_manager::get_switch_port()
{
    return switch_port;
}


void control_manager::set_xcpd_ip(std::string ip)
{
    xcpd_ip= ip;
    xcpd_addr= caddress(AF_INET, xcpd_ip.c_str(),xcpd_port);
}
std::string control_manager::get_xcpd_ip()
{
    return xcpd_ip;
}

void control_manager::set_xcpd_port(int p)
{
    xcpd_port=p;
    xcpd_addr= caddress(AF_INET, xcpd_ip.c_str(),xcpd_port);
}

int control_manager::get_xcpd_port()
{
    return xcpd_port;
}


void control_manager::set_higher_ip(std::string ip)
{
    higher_ip= ip;
    higher_addr= caddress(AF_INET, higher_ip.c_str(),higher_port);
}

std::string control_manager::get_higher_ip() {
    return higher_ip;
}

void control_manager::set_higher_port(int port) 
{
    higher_port= port;
    higher_addr= caddress(AF_INET, higher_ip.c_str(),higher_port);
}

int control_manager::get_higher_port() 
{
    return higher_port;
}

void control_manager::set_switch_name(std::string n)
{
    switch_name= n;
}
    
std::string control_manager::get_switch_name()
{
    return switch_name;
}

void control_manager::add_port(std::string n)
{
    port_names.push_back(n);
}

int control_manager::no_ports()
{
    return port_names.size();
}

int control_manager::get_port_no(std::string n)
{
    for (int i= 0; i < (int)port_names.size(); i++) {
        if (port_names[i] == n) {
            return i;
        }
    }
    return -1;
}

void control_manager::set_switch_to_xcpd_conn_passive()
{
    switch_to_xcpd_conn= PASSIVE_CONNECTION;
}
void control_manager::set_switch_to_xcpd_conn_active()
{
    switch_to_xcpd_conn= ACTIVE_CONNECTION;
}
     
bool control_manager::is_switch_to_xcpd_conn_active()
{
    return switch_to_xcpd_conn == ACTIVE_CONNECTION;
}

void control_manager::set_xcpd_to_control_conn_passive()
{
    xcpd_to_control_conn= PASSIVE_CONNECTION;
}
    
void control_manager::set_xcpd_to_control_conn_active()
{
    xcpd_to_control_conn= ACTIVE_CONNECTION;
}
        
bool control_manager::is_xcpd_to_control_conn_active()
{
    return xcpd_to_control_conn == ACTIVE_CONNECTION;
}



