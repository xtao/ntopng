--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

interface.find(ifname)
ifstats = interface.getStats()
is_historical = interface.isHistoricalInterface(interface.name2id(ifname))

sendHTTPHeader('text/html')
--sendHTTPHeader('application/json')

if(ifstats ~= nil) then
   uptime = ntop.getUptime()
   prefs = ntop.getPrefs()

-- Round up
   hosts_pctg = math.floor(1+((ifstats.stats_hosts*100)/prefs.max_num_hosts))
   flows_pctg = math.floor(1+((ifstats.stats_flows*100)/prefs.max_num_flows))
   aggregations_pctg = math.floor(1+((ifstats.stats_aggregations*100)/prefs.max_num_hosts))

   print('{ "ifname": "'.. ifname..'", "packets": '.. ifstats.stats_packets .. ', "bytes": ' .. ifstats.stats_bytes .. ', "drops": ' .. ifstats.stats_drops .. ', "alerts": '.. ntop.getNumQueuedAlerts() ..', "num_flows": '.. ifstats.stats_flows .. ', "num_hosts": ' .. ifstats.stats_hosts .. ', "num_aggregations": ' .. ifstats.stats_aggregations .. ', "epoch": ' .. os.time()..' , "uptime": " ' .. secondsToTime(uptime) .. '", "hosts_pctg": ' .. hosts_pctg .. ', "aggregations_pctg": ' .. aggregations_pctg .. ', "flows_pctg": ' .. flows_pctg)

   if(is_historical) then
    historical_stats = interface.getHistorical();
    if (historical_stats.on_load) then
      print(', "on_load": true')
    else
      print(', "on_load": false')
    end
    
    if((historical_stats.num_files ~= nil) and (historical_stats.num_files ~= 0))then
      success_file = historical_stats.num_files - historical_stats.open_error - historical_stats.file_error - historical_stats.query_error
      file_pctg = math.floor(1+((historical_stats.file_error*100)/historical_stats.num_files))
      open_pctg = math.floor(1+((historical_stats.open_error*100)/historical_stats.num_files))
      query_pctg = math.floor(1+((historical_stats.query_error*100)/historical_stats.num_files))
      success_pctg = math.floor(1+((success_file*100)/historical_stats.num_files))
      print(', "historical_if_name": "' .. historical_stats.interface_name ..'"') 
      print(', "historical_tot_files": "' .. historical_stats.num_files ..'"')
    else
      success_file = 0
      file_pctg = 0
      open_pctg = 0
      query_pctg = 0
      success_pctg = 0
    end
    
    print(', "success_file": ' .. success_file.. ', "open_error": ' .. historical_stats.open_error.. ', "file_error": '..historical_stats.file_error ..', "query_error": '..historical_stats.query_error)
    print(', "success_pctg": ' .. success_pctg..', "open_pctg": ' .. open_pctg.. ', "file_pctg": '.. file_pctg ..', "query_pctg": '..query_pctg)
  end
   print('}\n')
else
   print('{ }\n')
end
