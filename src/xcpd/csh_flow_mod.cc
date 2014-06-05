
#include <rofl/common/openflow/openflow_rofl_exceptions.h>
#include <rofl/common/openflow/openflow10.h>
#include <rofl/common/cerror.h>
#include <rofl/common/openflow/cofaction.h>
#include <rofl/common/utils/c_logger.h>

#include "csh_flow_mod.h"

morpheus::csh_flow_mod::csh_flow_mod(morpheus * parent, rofl::cofctl * const src, rofl::cofmsg_flow_mod * const msg ):chandlersession_base(parent, msg->get_xid()) {
	ROFL_DEBUG("%s called.\n", __PRETTY_FUNCTION__);
	process_flow_mod(src, msg);
}

bool morpheus::csh_flow_mod::process_flow_mod ( rofl::cofctl * const src, rofl::cofmsg_flow_mod * const msg ) {
	if(msg->get_version() != OFP10_VERSION) throw rofl::eBadVersion();
	ROFL_DEBUG("Incoming msg match: %s actions %s\n",
         msg->get_match().c_str(), msg->get_actions().c_str());
    switch (msg->get_command()) {
        case OFPFC_ADD:
            m_completed= morpheus::csh_flow_mod::process_add_flow(src,msg);
            return m_completed;
        case OFPFC_MODIFY:
            m_completed= morpheus::csh_flow_mod::process_modify_flow(src,msg);
            return m_completed;
        case OFPFC_MODIFY_STRICT:
            m_completed= morpheus::csh_flow_mod::process_modify_strict_flow(src,msg);
            return m_completed;
        case OFPFC_DELETE:
            m_completed= morpheus::csh_flow_mod::process_delete_flow(src,msg);
            return m_completed;
        case OFPFC_DELETE_STRICT:
            m_completed= morpheus::csh_flow_mod::process_delete_strict_flow(src,msg);
            return m_completed;           
        default:
            m_completed= true;
            return m_completed;
    }
    
    
    m_completed= true;
    return m_completed;
        ROFL_ERR ("FLOW_MOD command %s not supported. Dropping message\n",
            msg->c_str());
		m_completed = true;
		return m_completed;
	}
    

bool morpheus::csh_flow_mod::process_add_flow
    ( rofl::cofctl * const src, rofl::cofmsg_flow_mod * const msg )
{
    
    const cportvlan_mapper & mapper = m_parent->get_mapper();
	//struct ofp10_match * p = msg->get_match().ofpu.ofp10u_match;
    //dumpBytes( ROFL_DEBUG, (uint8_t *) p, sizeof(struct ofp10_match) );
	rofl::cflowentry entry(OFP10_VERSION);
    
	entry.set_command(msg->get_command());
	entry.set_idle_timeout(msg->get_idle_timeout());
	entry.set_hard_timeout(msg->get_hard_timeout());
	entry.set_cookie(msg->get_cookie());
	entry.set_priority(msg->get_priority());
	entry.set_buffer_id(msg->get_buffer_id());
	entry.set_out_port(msg->get_out_port());	
	entry.set_flags(msg->get_flags());
	entry.match = msg->get_match();
	entry.actions = msg->get_actions();
    
    rofl::cflowentry trans(OFP10_VERSION);
    try {
        trans= m_parent->get_fet()->trans_flow_entry(entry);
        m_parent->get_fet()->add_flow_entry(entry,trans);
        
    } catch (...) {
        m_parent->send_error_message( src, msg->get_xid(), OFP10ET_FLOW_MOD_FAILED,             OFP10FMFC_UNSUPPORTED, msg->soframe(), msg->framelen() );
        m_completed = true;
        return true;
    }
    
	rofl::cofaclist inlist = msg->get_actions();
	rofl::cofaclist outlist;
//	bool already_set_vlan = false;
	bool already_did_output = false;
// now translate the action and the match
	for(rofl::cofaclist::iterator a = inlist.begin(); a != inlist.end(); ++ a) {
		uint32_t supported_actions = m_parent->get_supported_actions();
		ROFL_DEBUG("%s supported actions by underlying switch found to be: %s.\n",
            __PRETTY_FUNCTION__,action_mask_to_string(supported_actions).c_str());
		if( ! ((1<<(be16toh(a->oac_header->type))) & supported_actions )) {
			// the action isn't supported by the underlying switch - complain - send an error message, then write to your MP. Start a Tea Party movement. Then start an Occupy OpenFlow group. If that still doesn't help become a recluse and blame the system.
			ROFL_DEBUG("Received a flow-mod with an unsupported action: %s %d\n", action_mask_to_string((1<<(be16toh(a->oac_header->type)))).c_str(),
             (1<<(be16toh(a->oac_header->type))));
			m_parent->send_error_message( src, msg->get_xid(), OFP10ET_FLOW_MOD_FAILED, OFP10FMFC_UNSUPPORTED, msg->soframe(), msg->framelen() );
			return true;
		}
		ROFL_DEBUG("Processing incoming action %s.\n",
            action_mask_to_string(1<<(be16toh(a->oac_header->type))).c_str());
		switch(be16toh(a->oac_header->type)) {
			case OFP10AT_OUTPUT: {
				uint16_t oport = be16toh(a->oac_10output->port);
				if ( ( oport == OFPP10_FLOOD ) || ( oport == OFPP10_ALL) ) {	// TODO check that the match isn't ALL
					// ALL is all except input port
					// FLOOD is all except input port and those disabled by STP.. which we don't support anyway - so I'm going to treat them the same way.
					// we need to generate a list of untagged output actions, then a list of tagged output actions for all interfaces except the input interface.
					rofl::cofaclist taggedoutputs;
					for(oport = 1; oport <= mapper.get_number_virtual_ports(); ++oport) {
						cportvlan_mapper::port_spec_t outport_spec = mapper.get_actual_port( oport );
						if(outport_spec.port == msg->get_match().get_in_port()) {
							// this is the input port - skipping
							continue;
						}
						ROFL_DEBUG("Generating output action from virtual port %d to actual %d\n",(unsigned) oport, outport_spec);
						if(outport_spec.vlanid_is_none())
							outlist.next() =  rofl::cofaction_output( OFP10_VERSION, outport_spec.port, be16toh(a->oac_10output->max_len) );
						else {
							taggedoutputs.next() = rofl::cofaction_set_vlan_vid( OFP10_VERSION, outport_spec.vlan);
							taggedoutputs.next() = rofl::cofaction_output( OFP10_VERSION, outport_spec.port, be16toh(a->oac_10output->max_len) );
						}
					}
					for(rofl::cofaclist::iterator toi = taggedoutputs.begin(); toi != taggedoutputs.end(); ++toi) outlist.next() = *toi;
					if(taggedoutputs.begin() != taggedoutputs.end()) outlist.next() = rofl::cofaction_strip_vlan( OFP10_VERSION );	// the input output action may not be the last such action so we need to clean up the VLAN that we've left on the "stack"
				} else {	// not FLOOD
					if(oport > mapper.get_number_virtual_ports() ) {
						// invalid virtual port number
						m_parent->send_error_message(src, msg->get_xid(), OFP10ET_BAD_ACTION, OFP10BAC_BAD_OUT_PORT, msg->soframe(), msg->framelen() );
						return true;
					}
					cportvlan_mapper::port_spec_t real_port = mapper.get_actual_port( oport );
					if(!real_port.vlanid_is_none()) {	// add a vlan tagger before an output if necessary
						outlist.next() = rofl::cofaction_set_vlan_vid( OFP10_VERSION, real_port.vlan );
//						already_set_vlan = true;
					}
					outlist.next() =  rofl::cofaction_output( OFP10_VERSION, real_port.port, be16toh(a->oac_10output->max_len) );	// add translated output action
				}
				already_did_output = true;
			} break;
			case OFP10AT_SET_VLAN_VID: {
				// VLAN-in-VLAN is not supported - return with error.
				assert(false);
				m_parent->send_error_message( src, msg->get_xid(), OFP10ET_FLOW_MOD_FAILED, OFP10FMFC_UNSUPPORTED, msg->soframe(), msg->framelen() );
				return true;
			} break;
			case OFP10AT_SET_VLAN_PCP: {
				// VLAN-in-VLAN is not supported - return with error.
				assert(false);
				m_parent->send_error_message( src, msg->get_xid(), OFP10ET_FLOW_MOD_FAILED, OFP10FMFC_UNSUPPORTED, msg->soframe(), msg->framelen() );
				m_completed = true;
				return true;
			} break;
			case OFP10AT_STRIP_VLAN: {
				if(already_did_output) {	// cannot strip after output has already been done
//				if(already_set_vlan) {
//					// cannot strip after we've already added a set-vlan message  JSP TODO - is this correct?
					ROFL_DEBUG ("%s attempt was made to strip VLAN after an OFP10AT_OUTPUT action. Rejecting flow-mod.", __PRETTY_FUNCTION__);
					m_parent->send_error_message( src, msg->get_xid(), OFP10ET_FLOW_MOD_FAILED, OFP10FMFC_UNSUPPORTED, msg->soframe(), msg->framelen() );
					return true;
				}
			} break;
			case OFP10AT_ENQUEUE: {
				// Queues not supported for now.
				assert(false);
				m_parent->send_error_message( src, msg->get_xid(), OFP10ET_FLOW_MOD_FAILED, OFP10FMFC_UNSUPPORTED, msg->soframe(), msg->framelen() );
				return true;
			} break;
			case OFP10AT_SET_DL_SRC:
			case OFP10AT_SET_DL_DST:
			case OFP10AT_SET_NW_SRC:
			case OFP10AT_SET_NW_DST:
			case OFP10AT_SET_NW_TOS:
			case OFP10AT_SET_TP_SRC:
			case OFP10AT_SET_TP_DST: {
				// just pass the message through
				outlist.next() = *a;
			} break;
			case OFP10AT_VENDOR: {
				// We have no idea what could be in the vendor message, so we can't translate, so we kill it.
				ROFL_DEBUG(" Vendor actions are unsupported. Sending error and dropping message."); 
				m_parent->send_error_message( src, msg->get_xid(), OFP10ET_FLOW_MOD_FAILED, OFP10FMFC_UNSUPPORTED, msg->soframe(), msg->framelen() );
				return true;
			} break;
			default:
			ROFL_DEBUG(" unknown action type (%d) Sending error and dropping message.\n",
                (unsigned)be16toh(a->oac_header->type));
			m_parent->send_error_message( src, msg->get_xid(), OFP10ET_FLOW_MOD_FAILED, OFP10FMFC_UNSUPPORTED, msg->soframe(), msg->framelen() );
			m_completed = true;
			return m_completed;
		}
	}
	entry.actions = outlist;
	
	rofl::cofmatch newmatch = msg->get_match();
	rofl::cofmatch oldmatch = newmatch;
	//check that VLANs are wildcarded (i.e. not being matched on)
	// TODO we *could* theoretically support incoming VLAN iff they are coming in on an port-translated-only port (i.e. a virtual port that doesn't map to a port+vlan, only a phsyical port), and that VLAN si then stripped in the action.
	try {
		oldmatch.get_vlan_vid_mask();	// ignore result - we only care if it'll throw
//		if(oldmatch.get_vlan_vid_mask() != 0xffff) {
        ROFL_DEBUG("Received a match which didn't have VLAN wildcarded. Sending error and dropping message. match: %s\n", msg->c_str());
        m_parent->send_error_message( src, msg->get_xid(), OFP10ET_FLOW_MOD_FAILED, OFP10FMFC_UNSUPPORTED, msg->soframe(), msg->framelen() );
		return true;
	} catch ( rofl::eOFmatchNotFound & ) {
		// do nothing - there was no vlan_vid_mask
	}
	// make sure this is a valid port
	// TODO check whether port is ANY/ALL
	uint32_t old_inport = oldmatch.get_in_port();
	try {
		cportvlan_mapper::port_spec_t real_port = mapper.get_actual_port( old_inport ); // could throw std::out_of_range
		if(!real_port.vlanid_is_none()) {
			// vlan is set in actual port - update the match
			newmatch.set_vlan_vid( real_port.vlan );
		}
		// update port
		newmatch.set_in_port( real_port.port );
	} catch (std::out_of_range &) {
		ROFL_DEBUG("%s: received a match request for an unknown port (%d) There are %d ports.  Sending error and dropping message. match:%s \n", 
            oldmatch.get_in_port(), mapper.get_number_virtual_ports() , msg->c_str());
        m_parent->send_error_message( src, msg->get_xid(), OFP10ET_FLOW_MOD_FAILED, OFP10FMFC_UNSUPPORTED, msg->soframe(), msg->framelen() );
		return true;
	}
	entry.match = newmatch;
	ROFL_DEBUG("Sending flow mod %s\n",entry.c_str());
	return true;
}

bool morpheus::csh_flow_mod::process_modify_flow
    ( rofl::cofctl * const src, rofl::cofmsg_flow_mod * const msg )
{
    ROFL_ERR ("FLOW_MOD command %s not supported. Dropping message\n",
        msg->c_str());
    return true;
}

bool morpheus::csh_flow_mod::process_modify_strict_flow
    ( rofl::cofctl * const src, rofl::cofmsg_flow_mod * const msg )
{
    ROFL_ERR ("FLOW_MOD command %s not supported. Dropping message\n",
        msg->c_str());
    return true;
}

bool morpheus::csh_flow_mod::process_delete_flow
    ( rofl::cofctl * const src, rofl::cofmsg_flow_mod * const msg )
{
    ROFL_ERR ("FLOW_MOD command %s not supported. Dropping message\n",
        msg->c_str());
    return true;
}

bool morpheus::csh_flow_mod::process_delete_strict_flow
    ( rofl::cofctl * const src, rofl::cofmsg_flow_mod * const msg )
{
    ROFL_ERR ("FLOW_MOD command %s not supported. Dropping message\n",
        msg->c_str());
    return true;
}

bool morpheus::csh_flow_mod::handle_error (rofl::cofdpt *src, rofl::cofmsg_error *msg) {
	ROFL_DEBUG("Warning: %s has received an error message: %s\n", __PRETTY_FUNCTION__, msg->c_str());
	m_completed = true;
	return m_completed;
}

morpheus::csh_flow_mod::~csh_flow_mod() { 
    ROFL_DEBUG ("%s called.\n", __PRETTY_FUNCTION__);
}


// std::string morpheus::csh_flow_mod::asString() const { return "csh_flow_mod {no xid}"; }
std::string morpheus::csh_flow_mod::asString() const { std::stringstream ss; ss << "csh_flow_mod {request_xid=" << m_request_xid << "}"; return ss.str(); }

