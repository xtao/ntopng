--
-- (C) 2013 - ntop.org
--

package.path = "./scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "template"
local json = require ("dkjson")

local debug_collector = ntop.verboseTrace()

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
[template.TCP_FLAGS]      = true, 
[template.PROTOCOL]       = true, 
[template.IN_PKTS]        = true, 
[template.IN_BYTES]       = true, 
[template.OUT_PKTS]       = true, 
[template.OUT_BYTES]      = true,
[template.FIRST_SWITCHED] = true, 
[template.LAST_SWITCHED]  = true
}

print("Starting ZMQ collector on "..ifname) 
interface.find(ifname)
local endpoint = interface.getEndpoint()
ntop.zmq_connect(endpoint, "flow")

print("ZMQ Collector connected to " .. endpoint .. "\n") 


while(interface.isRunning) do
  flowjson = ntop.zmq_receive()

  if(debug_collector) then print("[ZMQ] "..flowjson) end
  local flow, pos, err = json.decode(flowjson, 1, nil)
  if err then
    print("JSON parser error: " .. err)
  else

    if(false) then 
        for key,value in pairs(flow) do
	  print(key.."="..value) 
	end
    end

    local unhandled_fields = {}

    -- Flows can be sent with numeric ("PROTOCOL":6) or symbolic keys ("4":6)
    -- so we need to convert all of them, this to make sure they are uniform

    for _key,value in pairs(flow) do
        val = template[_key]
    	if(val ~= nil) then
	  key = val -- This was a symbolic key we convert to numeric
	  flow[key] = value
	else
	  key = _key
	end

      if not handled_fields[key] then	
        if rtemplate[key] ~= nil then
          unhandled_fields[rtemplate[key]] = value
        else
          unhandled_fields['"'..key..'"'] = value
        end
      end
    end

    local unhandled_fields_json = json.encode(unhandled_fields, {})

    interface.processFlow(
      flow[template.IN_SRC_MAC]     or "00:00:00:00:00:00",
      flow[template.OUT_DST_MAC]    or "00:00:00:00:00:00",
      flow[template.IPV4_SRC_ADDR]  or flow[template.IPV6_SRC_ADDR] or "0.0.0.0",
      flow[template.IPV4_DST_ADDR]  or flow[template.IPV6_DST_ADDR] or "0.0.0.0", 
      flow[template.L4_SRC_PORT]    or flow['L4_SRC_PORT'] or 0, 
      flow[template.L4_DST_PORT]    or 0, 
      flow[template.SRC_VLAN]       or flow[template.DST_VLAN] or 0, 
      flow[template.L7_PROTO]       or 0, 
      flow[template.PROTOCOL]       or 0, 
      flow[template.TCP_FLAGS]      or 0, 
      flow[template.IN_PKTS]        or 0, 
      flow[template.IN_BYTES]       or 0, 
      flow[template.OUT_PKTS]       or 0, 
      flow[template.OUT_BYTES]      or 0,
      flow[template.FIRST_SWITCHED] or 0, 
      flow[template.LAST_SWITCHED]  or 0,
      unhandled_fields_json         or "{}"
    )

    if debug_collector == 1 then
      print("Unhandled fields: " .. unhandled_fields_json)
    end
  end
end

ntop.zmq_disconnect()
