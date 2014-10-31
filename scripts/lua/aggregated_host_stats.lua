--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

host_ip = _GET["host"]

interface.find(ifname)
host = interface.getAggregatedHostInfo(host_ip)

sendHTTPHeader('text/html; charset=iso-8859-1')
--sendHTTPHeader('application/json')

print('{ "packets.rcvd": '.. host["packets.rcvd"] .. ', "last_seen": ' .. host["seen.last"] .. ', "epoch": ' .. os.time()..'" }\n')
