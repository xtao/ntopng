--
-- (C) 2013 - ntop.org
--

package.path = "./scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"
require "template"
local json = require ("dkjson")

local debug_collector = 1

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
        if rtemplate[key] ~= nil then
	  print(rtemplate[key] .. " = " .. value)
	else
          print("unknown field id " .. key .. " = " .. value)
	end
      end
      print("---")
    end

    interface.processFlow(
      flow[template.IN_SRC_MAC]     or "00:00:00:00:00:00",
      flow[template.OUT_DST_MAC]    or "00:00:00:00:00:00",
      flow[template.IPV4_SRC_ADDR]  or flow[template.IPV6_SRC_ADDR] or "0.0.0.0",
      flow[template.IPV4_DST_ADDR]  or flow[template.IPV6_DST_ADDR] or "0.0.0.0", 
      flow[template.L4_SRC_PORT]    or 0, 
      flow[template.L4_DST_PORT]    or 0, 
      flow[template.SRC_VLAN]       or flow[template.DST_VLAN] or 0, 
      flow[template.L7_PROTO]       or 0, 
      flow[template.PROTOCOL]       or 0, 
      flow[template.IN_PKTS]        or 0, 
      flow[template.IN_BYTES]       or 0, 
      flow[template.OUT_PKTS]       or 0, 
      flow[template.OUT_BYTES]      or 0,
      flow[template.FIRST_SWITCHED] or 0, 
      flow[template.LAST_SWITCHED]  or 0,
      ""
    )

  end

end

ntop.zmq_disconnect()

