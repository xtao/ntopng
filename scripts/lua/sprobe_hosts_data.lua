--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('application/json')

interface.find(ifname)
flows_stats = interface.getFlowsInfo()

links = {}

for key, value in pairs(flows_stats) do
   flow = flows_stats[key]

   if(flow["cli.host"] ~= nil) then c = flow["cli.host"] else c = flow["cli.ip"] end
   if(flow["srv.host"] ~= nil) then s = flow["srv.host"] else s = flow["srv.ip"] end
   
   c = c .. "@" .. flow["cli.source_id"]
   s = s .. "@" .. flow["srv.source_id"]

   if(flow["client_process"] ~= nil) then
      links[c] = s
   end

   if(flow["server_process"] ~= nil) then
      links[s] = c
   end
end

print("[")
n = 0
for key, value in pairs(links) do
   if(n > 0) then print(",") end

   print('\n{"source": "'..key..'", "source_name": "'..ntop.getResolvedAddress(key)..'", "target": "'..value..'", "target_name": "'.. ntop.getResolvedAddress(value)..'", "type": "suit"}')
   n = n + 1
end
print("\n]\n")

