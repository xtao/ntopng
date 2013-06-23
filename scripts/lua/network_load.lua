--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.workingdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

ifname = _GET["if"]
if(ifname == nil) then
   ifname = "any"
end

interface.find(ifname)
ifstats = interface.getStats()
info = ntop.getInfo()

sendHTTPHeader('text/html')
--sendHTTPHeader('application/json')

print('{ "packets": '.. ifstats.stats_packets .. ', "bytes": ' .. ifstats.stats_bytes .. ', "num_flows": '.. ifstats.stats_flows .. ', "num_hosts": ' .. ifstats.stats_hosts .. ', "epoch": ' .. os.time()..', "uptime": " ' .. secondsToTime(info["uptime"]) .. '" }\n')
