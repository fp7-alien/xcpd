
#include "csh_port_mod.h"
#include "control_manager.h"
#include <rofl/common/utils/c_logger.h>
#include <rofl/common/cerror.h>
#include <string>

using namespace xdpd;

// TODO translation check
morpheus::csh_port_mod::csh_port_mod(morpheus * parent, rofl::cofctl * const src, rofl::cofmsg_port_mod * const msg ):chandlersession_base(parent) {
	ROFL_DEBUG("%s called.\n",__PRETTY_FUNCTION__);
	process_port_mod(src, msg);
	}

bool morpheus::csh_port_mod::process_port_mod ( rofl::cofctl * const src, rofl::cofmsg_port_mod * const msg ) {
	if(msg->get_version() != OFP10_VERSION) throw rofl::eBadVersion();
    int type= control_manager::Instance()->get_port_config_handling();
    if (type == control_manager::DROP_COMMAND) {
        m_completed= true;
        return true;
    }
    if (type == control_manager::PASSTHROUGH_COMMAND) {
        const cportvlan_mapper & mapper = m_parent->get_mapper();
        cportvlan_mapper::port_spec_t real_port= 
            mapper.get_actual_port( msg->get_port_no() );
        uint32_t new_port= real_port.port;
        m_parent->send_port_mod_message( m_parent->get_dpt(), 
            new_port, msg->get_hwaddr(), 
            msg->get_config(), msg->get_mask(), 
            msg->get_advertise() );
        m_completed = true;
        return m_completed;
    }
    if (type == control_manager::HARDWARE_SPECIFIC_COMMAND) {
        hardware_manager *hwm= control_manager::Instance()->get_hardware_manager();
        if (hwm == NULL) {
            ROFL_ERR("No hardware manager defined for port-mod in %s\n",
                type, __PRETTY_FUNCTION__);
            throw rofl::eInval();
        }
        hwm->process_port_mod (src, msg );
        m_completed= true;
        return true;
    }
    ROFL_ERR("Undefined value for command type %d in %s\n",
        type, __PRETTY_FUNCTION__);
    throw rofl::eInval();
}

morpheus::csh_port_mod::~csh_port_mod() { 
    //std::cout << __FUNCTION__ << " called." << std::endl; #
}	// nothing to do as we didn't register anywhere.

std::string morpheus::csh_port_mod::asString() const { return "csh_port_mod {no xid}"; }
