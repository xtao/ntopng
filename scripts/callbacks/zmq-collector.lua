--
-- (C) 2013 - ntop.org
--

package.path = "./scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"
local json = require ("dkjson")

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

local ids = {}
for key,value in pairs(fields) do
  ids[value] = key
end

interface.find("zmq-collector")

local endpoint = interface.getEndpoint()

ntop.zmq_connect(endpoint, "flow")

print("ZMQ Collector connected to " .. endpoint .. "\n")

while(interface.isRunning) do
  flowjson = ntop.zmq_receive()

  local flow, pos, err = json.decode(flowjson, 1, nil)
  if err then
    print("JSON parser error: " .. err)
  else

    if debug_collector then
      for key,value in pairs(flow) do
        if fields[key] ~= nil then
	  print(fields[key] .. " = " .. value)
	else
          print("unknown field id " .. key .. " = " .. value)
	end
      end
      print("---")
    end

    interface.processFlow(
      flow[ids.IPV4_SRC_ADDR]  or flow[ids.IPV6_SRC_ADDR],
      flow[ids.IPV4_DST_ADDR]  or flow[ids.IPV6_DST_ADDR], 
      flow[ids.L4_SRC_PORT]    or 0, 
      flow[ids.L4_DST_PORT]    or 0, 
      flow[ids.SRC_VLAN]       or flow[ids.DST_VLAN] or 0, 
      flow[ids.APPLICATION_ID] or 0, 
      flow[ids.PROTOCOL]       or 0, 
      flow[ids.IN_PKTS]        or 0, 
      flow[ids.IN_BYTES]       or 0, 
      flow[ids.OUT_PKTS]       or 0, 
      flow[ids.OUT_BYTES]      or 0
    )

  end

end

ntop.zmq_disconnect()

