--
-- (C) 2013 - ntop.org
--

package.path = "./scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"
require "zmq"

local context = zmq.init(1)
local endpoint = "tcp://localhost:5556"
local topic = "flow"

print("Collecting flows...")

local subscriber = context:socket(zmq.SUB)

subscriber:connect(endpoint)
subscriber:setopt(zmq.SUBSCRIBE, topic)

--TODO stop condition
while 1 do
  local hdr = subscriber:recv()
  local msg = subscriber:recv()

  print("recvd flow: ", msg, "\n")

  --TODO process flow

end

subscriber:close()
context:term()

