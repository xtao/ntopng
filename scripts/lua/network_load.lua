--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

interface.find(ifname)
ifstats = interface.getStats()
info = ntop.getInfo()
prefs = ntop.getPrefs()

-- Round up
hosts_pctg = math.floor(1+((ifstats.stats_hosts*100)/prefs.max_num_hosts))
flows_pctg = math.floor(1+((ifstats.stats_flows*100)/prefs.max_num_flows))

sendHTTPHeader('text/html')
--sendHTTPHeader('application/json')

print('{ "packets": '.. ifstats.stats_packets .. ', "bytes": ' .. ifstats.stats_bytes .. ', "drops": ' .. ifstats.stats_drops .. ', "num_flows": '.. ifstats.stats_flows .. ', "num_hosts": ' .. ifstats.stats_hosts .. ', "epoch": ' .. os.time()..', "uptime": " ' .. secondsToTime(info["uptime"]) .. '", "hosts_pctg": ' .. hosts_pctg .. ', "flows_pctg": ' .. flows_pctg.. ' }\n')
