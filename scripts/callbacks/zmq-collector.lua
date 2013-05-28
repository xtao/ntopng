--
-- (C) 2013 - ntop.org
--

package.path = "./scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"
local json = require ("dkjson")

local endpoint = "tcp://127.0.0.1:5556"

local debug_collector = 1
local fields = {
[1] = "IN_BYTES",
[2] = "IN_PKTS",
[4] = "PROTOCOL",
[5] = "SRC_TOS",
[6] = "TCP_FLAGS",
[7] = "L4_SRC_PORT",
[8] = "IPV4_SRC_ADDR",
[9] = "IPV4_SRC_MASK",
[10] = "INPUT_SNMP",
[11] = "L4_DST_PORT",
[12] = "IPV4_DST_ADDR",
[13] = "IPV4_DST_MASK",
[14] = "OUTPUT_SNMP",
[15] = "IPV4_NEXT_HOP",
[16] = "SRC_AS",
[17] = "DST_AS",
[21] = "LAST_SWITCHED",
[22] = "FIRST_SWITCHED",
[23] = "OUT_BYTES",
[24] = "OUT_PKTS",
[27] = "IPV6_SRC_ADDR",
[28] = "IPV6_DST_ADDR",
[29] = "IPV6_SRC_MASK",
[30] = "IPV6_DST_MASK",
[58] = "SRC_VLAN",
[59] = "DST_VLAN",
[60] = "IP_PROTOCOL_VERSION",
[61] = "DIRECTION",
[95] = "APPLICATION_ID"
}

ntop.zmq_connect(endpoint, "flow")

print("Connected to " .. endpoint .. "\n")

interface.find(endpoint)

while(1) do
  flow = ntop.zmq_receive()

  local obj, pos, err = json.decode(flow, 1, nil)
  if err then
    print("JSON parser error: " .. err)
  else

    if debug_collector then
      for key,value in pairs(obj) do
        if fields[key] ~= nil then
	  print(fields[key] .. " === " .. value)
	else
          print("unknown field id " .. key .. " = " .. value)
	end
      end
    end
    print("---")

    if (obj[8]) then -- IPv4
      ips = obj[8]
      ipd = obj[12]
    else -- IPv6
      ips = obj[27]
      ipd = obj[28]
    end
    srcport  = obj[7]  or 0;
    dstport  = obj[11] or 0; 
    vlanid   = obj[58] or 0; 
    protoid  = obj[95] or 0; 
    l4proto  = obj[4]  or 0; 
    inpkts   = obj[2]  or 0; 
    inbytes  = obj[1]  or 0;
    outpkts  = obj[24] or 0; 
    outbytes = obj[23] or 0;

    interface.processFlow(ips, ipd, srcport, dstport, vlanid, protoid, l4proto, inpkts, inbytes, outpkts, outbytes);
  end

end

ntop.zmq_disconnect()

