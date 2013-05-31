--
-- (C) 2013 - ntop.org
--

function handleCustomFlowField(key, value)
   if(key == 'TCP_FLAGS') then
      return(formatTcpFlags(value))
      elseif((key == 'INPUT_SNMP') or (key == 'OUTPUT_SNMP')) then
      return(formatInterfaceId(value))
   end

   return value
end

-- #######################

function formatTcpFlags(flags)  
   if(flags == 0) then
      return("")
   end

   rsp = "<A HREF=http://en.wikipedia.org/wiki/Transmission_Control_Protocol>"
   if(bit.band(flags, 1) == 2)  then rsp = rsp .. " SYN "  end
   if(bit.band(flags, 16) == 16) then rsp = rsp .. " ACK "  end
   if(bit.band(flags, 1) == 1)  then rsp = rsp .. " FIN "  end
   if(bit.band(flags, 4) == 4)  then rsp = rsp .. " RST "  end
   if(bit.band(flags, 8) == 8 )  then rsp = rsp .. " PUSH " end

   
   return(rsp .. "</A>")
end

-- #######################

function formatInterfaceId(id)  
   if(id == 65535) then
      return("Unknown")
   else
      return(id)
   end
end


-- #######################

flow_fields_description = {
   IN_BYTES = "Incoming flow bytes (src->dst)",
   IN_PKTS = "Incoming flow packets (src->dst)",
   FLOWS = "Number of flows",
   PROTOCOL = "IP protocol byte",
   PROTOCOL_MAP = "IP protocol name",
   SRC_TOS = "Type of service byte",
   TCP_FLAGS = "Cumulative of all flow TCP flags",
   L4_SRC_PORT = "IPv4 source port",
   L4_SRC_PORT_MAP = "Layer 4 source port symbolic name",
   IPV4_SRC_ADDR = "IPv4 source address",
   IPV4_SRC_MASK = "IPv4 source subnet mask (/<bits>)",
   INPUT_SNMP = "Input interface SNMP idx",
   L4_DST_PORT = "IPv4 destination port",
   L4_DST_PORT_MAP = "Layer 4 destination port symbolic name",
   L4_SRV_PORT = "Layer 4 server port",
   L4_SRV_PORT_MAP = "Layer 4 server port symbolic name",
   IPV4_DST_ADDR = "IPv4 destination address",
   IPV4_DST_MASK = "IPv4 dest subnet mask (/<bits>)",
   OUTPUT_SNMP = "Output interface SNMP idx",
   IPV4_NEXT_HOP = "IPv4 next hop address",
   SRC_AS = "Source BGP AS",
   DST_AS = "Destination BGP AS",
   LAST_SWITCHED = "SysUptime (msec) of the last flow pkt",
   FIRST_SWITCHED = "SysUptime (msec) of the first flow pkt",
   OUT_BYTES = "Outgoing flow bytes (dst->src)",
   OUT_PKTS = "Outgoing flow packets (dst->src)",
   IPV6_SRC_ADDR = "IPv6 source address",
   IPV6_DST_ADDR = "IPv6 destination address",
   IPV6_SRC_MASK = "IPv6 source mask",
   IPV6_DST_MASK = "IPv6 destination mask",
   ICMP_TYPE = "ICMP Type * 256 + ICMP code",
   SAMPLING_INTERVAL = "Sampling rate",
   SAMPLING_ALGORITHM = "Sampling type (deterministic/random)",
   FLOW_ACTIVE_TIMEOUT = "Activity timeout of flow cache entries",
   FLOW_INACTIVE_TIMEOUT = "Inactivity timeout of flow cache entries",
   ENGINE_TYPE = "Flow switching engine",
   ENGINE_ID = "Id of the flow switching engine",
   TOTAL_BYTES_EXP = "Total bytes exported",
   TOTAL_PKTS_EXP = "Total flow packets exported",
   TOTAL_FLOWS_EXP = "Total number of exported flows",
   MIN_TTL = "Min flow TTL",
   MAX_TTL = "Max flow TTL",
   IN_SRC_MAC = "Source MAC Address",
   SRC_VLAN = "Source VLAN",
   DST_VLAN = "Destination VLAN",
   IP_PROTOCOL_VERSION = "[4=IPv4][6=IPv6]",
   DIRECTION = "It indicates where a sample has been taken (always 0)",
   IPV6_NEXT_HOP = "IPv6 next hop address",
   MPLS_LABEL_1 = "MPLS label at position 1",
   MPLS_LABEL_2 = "MPLS label at position 2",
   MPLS_LABEL_3 = "MPLS label at position 3",
   MPLS_LABEL_4 = "MPLS label at position 4",
   MPLS_LABEL_5 = "MPLS label at position 5",
   MPLS_LABEL_6 = "MPLS label at position 6",
   MPLS_LABEL_7 = "MPLS label at position 7",
   MPLS_LABEL_8 = "MPLS label at position 8",
   MPLS_LABEL_9 = "MPLS label at position 9",
   MPLS_LABEL_10 = "MPLS label at position 10",
   OUT_DST_MAC = "Destination MAC Address",
   APPLICATION_ID = "Cisco NBAR Application Id",
   PACKET_SECTION_OFFSET = "Packet section offset",
   SAMPLED_PACKET_SIZE = "Sampled packet size",
   SAMPLED_PACKET_ID = "Sampled packet id",
   EXPORTER_IPV4_ADDRESS = "Exporter IPv4 Address",
   EXPORTER_IPV6_ADDRESS = "Exporter IPv6 Address",
   FLOW_ID = "Serial Flow Identifier",
   FLOW_START_SEC = "Seconds (epoch) of the first flow packet",
   FLOW_END_SEC = "Seconds (epoch) of the last flow packet",
   FLOW_START_MILLISECONDS = "Msec (epoch) of the first flow packet",
   FLOW_END_MILLISECONDS = "Msec (epoch) of the last flow packet",
   BIFLOW_DIRECTION = "1=initiator, 2=reverseInitiator",
   OBSERVATION_POINT_TYPE = "Observation point type",
   OBSERVATION_POINT_ID = "Observation point id",
   SELECTOR_ID = "Selector id",
   IPFIX_SAMPLING_ALGORITHM = "Sampling algorithm",
   SAMPLING_SIZE = "Number of packets to sample",
   SAMPLING_POPULATION = "Sampling population",
   FRAME_LENGTH = "Original L2 frame length",
   PACKETS_OBSERVED = "Tot number of packets seen",
   PACKETS_SELECTED = "Number of pkts selected for sampling",
   SELECTOR_NAME = "Sampler name",
   FRAGMENTS = "Number of fragmented flow packets",
   CLIENT_NW_DELAY_SEC = "Network latency client <-> nprobe (sec) [deprecated]",
   CLIENT_NW_DELAY_USEC = "Network latency client <-> nprobe (residual usec) [deprecated]",
   CLIENT_NW_DELAY_MS = "Network latency client <-> nprobe (msec)",
   SERVER_NW_DELAY_SEC = "Network latency nprobe <-> server (sec) [deprecated]",
   SERVER_NW_DELAY_USEC = "Network latency nprobe <-> server (residual usec) [deprecated]",
   SERVER_NW_DELAY_MS = "Network latency nprobe <-> server (residual msec)",
   APPL_LATENCY_SEC = "Application latency (sec) [deprecated]",
   APPL_LATENCY_USEC = "Application latency (residual usec) [deprecated]",
   APPL_LATENCY_MS = "Application latency (msec)",
   NUM_PKTS_UP_TO_128_BYTES = "# packets whose size <= 128",
   NUM_PKTS_128_TO_256_BYTES = "# packets whose size > 128 and <= 256",
   NUM_PKTS_256_TO_512_BYTES = "# packets whose size > 256 and < 512",
   NUM_PKTS_512_TO_1024_BYTES = "# packets whose size > 512 and < 1024",
   NUM_PKTS_1024_TO_1514_BYTES = "# packets whose size > 1024 and <= 1514",
   NUM_PKTS_OVER_1514_BYTES = "# packets whose size > 1514",
   CUMULATIVE_ICMP_TYPE = "Cumulative OR of ICMP type packets",
   SRC_IP_COUNTRY = "Country where the src IP is located",
   SRC_IP_CITY = "City where the src IP is located",
   DST_IP_COUNTRY = "Country where the dst IP is located",
   DST_IP_CITY = "City where the dst IP is located",
   FLOW_PROTO_PORT = "L7 port that identifies the flow protocol or 0 if unknown",
   UPSTREAM_TUNNEL_ID = "Upstream tunnel identifier (e.g. GTP TEID) or 0 if unknown",
   LONGEST_FLOW_PKT = "Longest packet (bytes) of the flow",
   SHORTEST_FLOW_PKT = "Shortest packet (bytes) of the flow",
   RETRANSMITTED_IN_PKTS = "Number of retransmitted TCP flow packets (src->dst)",
   RETRANSMITTED_OUT_PKTS = "Number of retransmitted TCP flow packets (dst->src)",
   OOORDER_IN_PKTS = "Number of out of order TCP flow packets (dst->src)",
   OOORDER_OUT_PKTS = "Number of out of order TCP flow packets (dst->src)",
   UNTUNNELED_PROTOCOL = "Untunneled IP protocol byte",
   UNTUNNELED_IPV4_SRC_ADDR = "Untunneled IPv4 source address",
   UNTUNNELED_L4_SRC_PORT = "Untunneled IPv4 source port",
   UNTUNNELED_IPV4_DST_ADDR = "Untunneled IPv4 destination address",
   UNTUNNELED_L4_DST_PORT = "Untunneled IPv4 destination port",
   L7_PROTO = "Layer 7 protocol (numeric)",
   L7_PROTO_NAME = "Layer 7 protocol name",
   DOWNSTREAM_TUNNEL_ID = "Downstream tunnel identifier (e.g. GTP TEID) or 0 if unknown",
   FLOW_USER_NAME = "Flow username of the tunnel (if known)",
   FLOW_SERVER_NAME = "Flow server name (if known)",
   PLUGIN_NAME = "Plugin name used by this flow (if any)",
   NUM_PKTS_TTL_EQ_1 = "# packets with TTL = 1",
   NUM_PKTS_TTL_2_5 = "# packets with TTL > 1 and TTL <= 5",
   NUM_PKTS_TTL_5_32 = "# packets with TTL > 5 and TTL <= 32",
   NUM_PKTS_TTL_32_64 = "# packets with TTL > 32 and <= 64 ",
   NUM_PKTS_TTL_64_96 = "# packets with TTL > 64 and <= 96",
   NUM_PKTS_TTL_96_128 = "# packets with TTL > 96 and <= 128",
   NUM_PKTS_TTL_128_160 = "# packets with TTL > 128 and <= 160",
   NUM_PKTS_TTL_160_192 = "# packets with TTL > 160 and <= 192",
   NUM_PKTS_TTL_192_224 = "# packets with TTL > 192 and <= 224",
   NUM_PKTS_TTL_224_255 = "# packets with TTL > 224 and <= 255",
   IN_SRC_OSI_SAP = "OSI Source SAP (OSI Traffic Only)",
   OUT_DST_OSI_SAP = "OSI Destination SAP (OSI Traffic Only)",
   MYSQL_SERVER_VERSION = "MySQL server version",
   MYSQL_USERNAME = "MySQL username",
   MYSQL_DB = "MySQL database in use",
   MYSQL_QUERY = "MySQL Query",
   MYSQL_RESPONSE = "MySQL server response",
   MYSQL_APPL_LATENCY_USEC = "MySQL request->response latecy (usec)",
   SRC_AS_PATH_1 = "Src AS path position 1",
   SRC_AS_PATH_2 = "Src AS path position 2",
   SRC_AS_PATH_3 = "Src AS path position 3",
   SRC_AS_PATH_4 = "Src AS path position 4",
   SRC_AS_PATH_5 = "Src AS path position 5",
   SRC_AS_PATH_6 = "Src AS path position 6",
   SRC_AS_PATH_7 = "Src AS path position 7",
   SRC_AS_PATH_8 = "Src AS path position 8",
   SRC_AS_PATH_9 = "Src AS path position 9",
   SRC_AS_PATH_10 = "Src AS path position 10",
   DST_AS_PATH_1 = "Dest AS path position 1",
   DST_AS_PATH_2 = "Dest AS path position 2",
   DST_AS_PATH_3 = "Dest AS path position 3",
   DST_AS_PATH_4 = "Dest AS path position 4",
   DST_AS_PATH_5 = "Dest AS path position 5",
   DST_AS_PATH_6 = "Dest AS path position 6",
   DST_AS_PATH_7 = "Dest AS path position 7",
   DST_AS_PATH_8 = "Dest AS path position 8",
   DST_AS_PATH_9 = "Dest AS path position 9",
   DST_AS_PATH_10 = "Dest AS path position 10",
   GTPV1_REQ_MSG_TYPE = "GTPv1 Request Msg Type",
   GTPV1_RSP_MSG_TYPE = "GTPv1 Response Msg Type",
   GTPV1_C2S_TEID_DATA = "GTPv1 Client->Server TunnelId Data",
   GTPV1_C2S_TEID_CTRL = "GTPv1 Client->Server TunnelId Control",
   GTPV1_S2C_TEID_DATA = "GTPv1 Server->Client TunnelId Data",
   GTPV1_S2C_TEID_CTRL = "GTPv1 Server->Client TunnelId Control",
   GTPV1_END_USER_IP = "GTPv1 End User IP Address",
   GTPV1_END_USER_IMSI = "GTPv1 End User IMSI",
   GTPV1_END_USER_MSISDN = "GTPv1 End User MSISDN",
   GTPV1_END_USER_IMEI = "GTPv1 End User IMEI",
   GTPV1_APN_NAME = "GTPv1 APN Name",
   GTPV1_RAI_MCC = "GTPv1 RAI Mobile Country Code",
   GTPV1_RAI_MNC = "GTPv1 RAI Mobile Network Code",
   GTPV1_RAI_LAC = "GTPv1 RAI Location Area Code",
   GTPV1_RAI_RAC = "GTPv1 RAI Routing Area Code",
   GTPV1_ULI_MCC = "GTPv1 ULI Mobile Country Code",
   GTPV1_ULI_MNC = "GTPv1 ULI Mobile Network Code",
   GTPV1_ULI_CELL_LAC = "GTPv1 ULI Cell Location Area Code",
   GTPV1_ULI_CELL_CI = "GTPv1 ULI Cell CI",
   GTPV1_ULI_SAC = "GTPv1 ULI SAC",
   GTPV1_RESPONSE_CAUSE = "GTPv1 Cause of Operation",
   HTTP_URL = "HTTP URL",
   HTTP_RET_CODE = "HTTP return code (e.g. 200, 304...)",
   HTTP_REFERER = "HTTP Referer",
   HTTP_UA = "HTTP User Agent",
   HTTP_MIME = "HTTP Mime Type",
   HTTP_HOST = "HTTP Host Name",
   HTTP_FBOOK_CHAT = "HTTP Facebook Chat"
}

-- #######################

function getFlowKey(name)
   for key,value in pairs(flow_fields_description) do
      if(key == name) then
	 return(value)
      end
   end

   return(name)
end

-- #######################

