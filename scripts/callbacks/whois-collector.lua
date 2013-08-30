--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "template"
local json = require ("dkjson")

local debug_collector = 0

local handled_fields = { 
[template.IN_SRC_MAC]     = true,
[template.OUT_DST_MAC]    = true,
[template.IPV4_SRC_ADDR]  = true,
[template.IPV4_SRC_MASK]  = true,
[template.IPV4_DST_ADDR]  = true,
[template.IPV4_DST_MASK]  = true,
[template.IPV6_SRC_ADDR]  = true,
[template.IPV6_DST_ADDR]  = true,
[template.L4_SRC_PORT]    = true,
[template.L4_DST_PORT]    = true,
[template.SRC_VLAN]       = true,
[template.DST_VLAN]       = true,
[template.L7_PROTO]       = true, 
[template.L7_PROTO_NAME]  = true, 
[template.PROTOCOL]       = true, 
[template.IN_PKTS]        = true, 
[template.IN_BYTES]       = true, 
[template.OUT_PKTS]       = true, 
[template.OUT_BYTES]      = true,
[template.FIRST_SWITCHED] = true, 
[template.LAST_SWITCHED]  = true
}
 
interface.find("zmq-collector")
local endpoint = interface.getEndpoint()
ntop.zmq_connect(endpoint, "whois")

print("ZMQ Collector connected to " .. endpoint .. "\n")

while(interface.isRunning) do
  flowjson = ntop.zmq_receive()

  local flow, pos, err = json.decode(flowjson, 1, nil)
  if err then
    print("JSON parser error: " .. err)
  else

    local unhandled_fields = {}

    for key,value in pairs(flow) do
      if not handled_fields[key] then
        if rtemplate[key] ~= nil then
          unhandled_fields[rtemplate[key]] = value
        else
          unhandled_fields['"'..key..'"'] = value
        end
      end

      if debug_collector == 1 then
        if rtemplate[key] ~= nil then
	  print(rtemplate[key] .. " = " .. value)
        else
          print("unknown field id " .. key .. " = " .. value)
	end
      end
    end

    local unhandled_fields_json = json.encode(unhandled_fields, {})

    interface.processFlow(
      flow[template.IN_SRC_MAC]     or "00:00:00:00:00:00",
      flow[template.OUT_DST_MAC]    or "00:00:00:00:00:00",
      flow[template.IPV4_SRC_ADDR]  or "0.0.0.0",
      flow[template.IPV4_DST_ADDR]  or "0.0.0.0", 
      flow[template.L4_SRC_PORT]    or 0, 
      flow[template.L4_DST_PORT]    or 0, 
      flow[template.SRC_VLAN]       or flow[template.DST_VLAN] or 0, 
      flow[template.L7_PROTO]       or 0, 
      flow[template.PROTOCOL]       or 6, 
      flow[template.IN_PKTS]        or 0, 
      flow[template.IN_BYTES]       or 0, 
      flow[template.OUT_PKTS]       or 0, 
      flow[template.OUT_BYTES]      or 0,
      flow[template.FIRST_SWITCHED] or 0, 
      flow[template.LAST_SWITCHED]  or 0,
      unhandled_fields_json         or "{}"
    )

    if debug_collector == 1 then
      print("unhandled fields: " .. unhandled_fields_json)
      print("---")
    end
  end
end

ntop.zmq_disconnect()

