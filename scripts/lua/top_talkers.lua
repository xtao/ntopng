--
-- (C) 2013 - ntop.org
--
package.path = "./scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"

tracked_host = _GET["host"]

ifname = _GET["if"]
interface.find("any")

interface.find(ifname)
hosts_stats = interface.getFlowsInfo()

sent = {}
_sent = {}
rcvd = {}
_rcvd = {}

for _key, value in pairs(hosts_stats) do
   key = hosts_stats[_key]["src.ip"]   
   old = _sent[key]
   if(old == nil) then old = 0 end
   _sent[key] = old + hosts_stats[_key]["cli2srv.bytes"]

   key = hosts_stats[_key]["dst.ip"]
   old = _rcvd[key]
   if(old == nil) then old = 0 end
   _rcvd[key] = old + hosts_stats[_key]["cli2srv.bytes"]

   -- ###########################

   key = hosts_stats[_key]["dst.ip"]   
   old = _sent[key]
   if(old == nil) then old = 0 end
   _sent[key] = old + hosts_stats[_key]["srv2cli.bytes"]

   key = hosts_stats[_key]["src.ip"]
   old = _rcvd[key]
   if(old == nil) then old = 0 end
   _rcvd[key] = old + hosts_stats[_key]["srv2cli.bytes"]
end

for key, value in pairs(_sent) do
   sent[value] = key
end

for key, value in pairs(_rcvd) do
   rcvd[value] = key
end


num = 0
for _key, _value in pairsByKeys(sent, rev) do
   key   = sent[_key]
   value = _key

   print(key..": "..value.."\n")
   num = num + 1

   if(num == 10) then
      break
   end
end

print("\n")

num = 0
for _key, _value in pairsByKeys(rcvd, rev) do
   key   = rcvd[_key]
   value = _key

   print(key..": "..value.."\n")
   num = num + 1

   if(num == 10) then
      break
   end
end

