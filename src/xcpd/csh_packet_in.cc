
#include "csh_packet_in.h"
#include <rofl/common/utils/c_logger.h>

morpheus::csh_packet_in::csh_packet_in(morpheus * parent, const rofl::cofdpt * const src, rofl::cofmsg_packet_in * const msg ):chandlersession_base(parent, msg->get_xid()) {
	std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
	process_packet_in(src, msg);
	}

bool morpheus::csh_packet_in::process_packet_in ( const rofl::cofdpt * const src, rofl::cofmsg_packet_in * const msg ) {
	if(msg->get_version() != OFP10_VERSION) throw rofl::eBadVersion();
	rofl::cofctl * master = m_parent->get_ctl();
	std::cout << "** BEFORE:" << std::endl;
	rofl::cofmatch match(msg->get_match_const());	
	std::cout << "match found to be " << match.c_str() << std::endl;	
	rofl::cpacket packet(msg->get_packet_const());
///	packet.get_match().set_in_port(msg->get_in_port());	// JSP: this is unnecessary as packet.get_match is locally generated anyway.


// extract the VLAN from the incoming packet
int32_t in_port = -1;
int32_t in_vlan = -1;
std::cout << "GETTING VLAN " << std::endl;
if(packet.cnt_vlan_tags()>0) {	// check to see if we have a vlan header
	in_vlan = packet.vlan(0)->get_dl_vlan_id();
	std::cout << "NO TAGS " << packet.cnt_vlan_tags() << " TAG " << in_vlan << std::endl;
	if(in_vlan == 0xffff) in_vlan = -1;	// it would be weird for this to happen - there's a vlan frame, but no tag. not sure if it's possible.
}
in_port = msg->get_in_port();

const cportvlan_mapper & mapper = m_parent->get_mapper();

cportvlan_mapper::port_spec_t inport_spec( PV_PORT_T(in_port), (in_vlan==-1)?(PV_VLANID_T::NONE):(PV_VLANID_T(in_vlan)) );

std::vector<std::pair<uint16_t, cportvlan_mapper::port_spec_t> > vports = mapper.actual_to_virtual_map( inport_spec );
ROFL_DEBUG ("%s: packet on port %ld vlan %ld  with %d virtual ports\n",
	__PRETTY_FUNCTION__, in_port, in_vlan, 
		vports.size());
for(std::vector<std::pair<uint16_t, cportvlan_mapper::port_spec_t> >::const_iterator ci=vports.begin(); ci != vports.end(); ++ci) std::cout << " " << (unsigned)ci->first;
std::cout << std::endl;

if(vports.size() != 1) {	// TODO handle this better
	std::cout << __FUNCTION__ << ": Incoming packet on port spec which doesn't match a virtual port. Dropping." << std::endl;
	m_completed = true;
	return m_completed;
}
// std::cout << "TP" << __LINE__ << std::endl; 
std::pair<uint16_t, cportvlan_mapper::port_spec_t> vport = vports.front();

// required changes to be made to packet done by this code:
// Strip VLAN on incoming packet
// rewrite get_in_port
// fiddle match struct

if(in_vlan != -1) packet.pop_vlan();	// remove the first VLAN header
// std::cout << "TP" << __LINE__ << std::endl;
// what to do with match??
// ANSWER - NOTHING. It's not used in OF10..

	m_parent->send_packet_in_message(master, msg->get_buffer_id(), msg->get_total_len(), msg->get_reason(), 0, 0, vport.first, match, packet.soframe(), packet.framelen() );	// TODO - the length fields are guesses.
//	m_parent->send_packet_in_message(master, msg->get_buffer_id(), msg->get_total_len(), msg->get_reason(), 0, 0, msg->get_in_port(), match, packet.ether()->sopdu(), packet.framelen() );	// TODO - the length fields are guesses.
	std::cout << __FUNCTION__ << " : packet_in forwarded to " << master->c_str() << "." << std::endl;
	m_completed = true;
	return m_completed;
}

bool morpheus::csh_packet_in::handle_error (rofl::cofdpt *src, rofl::cofmsg_error *msg) {
	ROFL_DEBUG("Warning: %s has received an error message: %s\n", __PRETTY_FUNCTION__, msg->c_str());
	m_completed = true;
	return m_completed;
}

morpheus::csh_packet_in::~csh_packet_in() { std::cout << __FUNCTION__ << " called." << std::endl; }	// nothing to do as we didn't register anywhere.

// std::string morpheus::csh_packet_in::asString() const { return "csh_packet_in {no xid}"; }

std::string morpheus::csh_packet_in::asString() const { std::stringstream ss; ss << "csh_packet_in {request_xid=" << m_request_xid << "}"; return ss.str(); }
