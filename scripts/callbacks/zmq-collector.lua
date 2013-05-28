--
-- (C) 2013 - ntop.org
--

package.path = "./scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"
local json = require ("dkjson")

-- nprobe --zmq tcp://127.0.0.1:5556

ntop.zmq_connect("tcp://127.0.0.1:5556", "flow")

print("Connected\n")

i = 0
while(i < 5) do
  flow = ntop.zmq_receive()
  print("Flow JSON: " .. flow .. "\n")

  local obj, pos, err = json.decode(flow, 1, nil)
  if err then
    print("Error:", err)
  else
    for key,value in pairs(obj) do
      print("[" .. key .. "=" .. value .. "]")
    end
  end

  i = i + 1
end

ntop.zmq_disconnect()

