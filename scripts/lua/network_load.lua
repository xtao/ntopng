--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

interface.find(ifname)
ifstats = interface.getStats()

sendHTTPHeader('text/html')
--sendHTTPHeader('application/json')

if(ifstats ~= nil) then
   uptime = ntop.getUptime()
   prefs = ntop.getPrefs()
   
-- Round up
   hosts_pctg = math.floor(1+((ifstats.stats_hosts*100)/prefs.max_num_hosts))
   flows_pctg = math.floor(1+((ifstats.stats_flows*100)/prefs.max_num_flows))
   aggregations_pctg = math.floor(1+((ifstats.stats_aggregations*100)/prefs.max_num_hosts))
   
   print('{ "packets": '.. ifstats.stats_packets .. ', "bytes": ' .. ifstats.stats_bytes .. ', "drops": ' .. ifstats.stats_drops .. ', "alerts": '.. ntop.getNumQueuedAlerts() ..', "num_flows": '.. ifstats.stats_flows .. ', "num_hosts": ' .. ifstats.stats_hosts .. ', "num_aggregations": ' .. ifstats.stats_aggregations .. ', "epoch": ' .. os.time()..' , "uptime": " ' .. secondsToTime(uptime) .. '", "hosts_pctg": ' .. hosts_pctg .. ', "aggregations_pctg": ' .. aggregations_pctg .. ', "flows_pctg": ' .. flows_pctg.. ' }\n')
else
   print('{ }\n')
end