--
-- (C) 2014 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html; charset=iso-8859-1')

host_ip = _GET["host"]

interface.find(ifname)
host = interface.getHostInfo(host_ip)

if(host == nil) then
   value = 0
else
   value = host["pkts.sent"]+host["pkts.rcvd"]
end

print(' { "value": ' .. value .. ' } ')