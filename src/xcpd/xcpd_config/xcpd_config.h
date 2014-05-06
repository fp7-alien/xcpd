#ifndef XCPD_CONFIG_H
#define XCPD_CONFIG_H 

#include <iostream>
#include <libconfig.h++> 
#include <rofl/platform/unix/cunixenv.h>
#include <rofl/common/cerror.h>
#include "../../xdpd/management/plugins/config/scope.h"
/**
* @file xcpd_config.h
* @author Richard Clegg richard@richardclegg.org
* @brief Config for xcpd
* 
*/

namespace xdpd {

class xcpd_config {
    
public:
    void init(int args, char** argv);
private:
    void parse_config(libconfig::Config* cfg, rofl::cunixenv& env_parser);	
};

class xcpd_root_scope : public scope {
	
public:
	xcpd_root_scope();
	virtual ~xcpd_root_scope() {};
		
private:

};

class xcpd_config_scope : public scope {
	
public:
	xcpd_config_scope();
	virtual ~xcpd_config_scope() {};
		

};

class xcpd_interfaces_scope:public scope {
	
public:
	xcpd_interfaces_scope(std::string scope_name="interfaces", bool mandatory=false);

	
};

class xcpd_virtual_ifaces_scope:public scope {
	
public:
	xcpd_virtual_ifaces_scope(std::string scope_name="virtual", bool mandatory=false);
		
};

class xcpd_openflow_scope:public scope {
	
public:
	xcpd_openflow_scope(std::string scope_name="openflow", bool mandatory=true);
		
protected:
	
};

class xcpd_of_lsis_scope:public scope {
	
public:
	xcpd_of_lsis_scope(std::string scope_name="logical-switches", bool mandatory=true);
		
};

class xcpd_lsi_scope:public scope {
	
public:
	xcpd_lsi_scope(std::string scope_name, bool mandatory=false);
};
}// namespace xdpd 

#endif /* XCPD_CONFIG_H_ */