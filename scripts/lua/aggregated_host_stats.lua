--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

host_ip = _GET["host"]

interface.find(ifname)
host = interface.getAggregatedHostInfo(host_ip)

sendHTTPHeader('text/html')
--sendHTTPHeader('application/json')

print('{ "pkts.rcvd": '.. host["pkts.rcvd"] .. ', "last_seen": ' .. host["seen.last"] .. ', "epoch": ' .. os.time()..'" }\n')
