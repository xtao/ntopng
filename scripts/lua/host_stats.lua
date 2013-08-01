--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "flow_utils"


local json = require ("dkjson")

ifname = _GET["if"]
if(ifname == nil) then	  
  ifname = "any"
end

host_ip = _GET["host"]
if(host_ip ~= nil) then
   interface.find(ifname)
   host = interface.getHostInfo(host_ip)
else
   host = "{}"
end


sendHTTPHeader('text/html')
--sendHTTPHeader('application/json')

print(json.encode(host, { indent = true }))
