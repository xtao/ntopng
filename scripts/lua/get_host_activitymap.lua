--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "graph_utils"

host_ip = _GET["host"]

sendHTTPHeader('application/json')

interface.find(ifname)

if((_GET["aggregated"] == nil) or (_GET["aggregated"] == 0)) then
   aggregation = false
   --print("false")
else
   aggregation = true
   --print("true")
end

rsp = interface.getHostActivityMap(host_ip, aggregation)
--print (host_ip)
print(rsp)