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
rsp = interface.getHostActivityMap(host_ip)
--print (host_ip)
print(rsp)