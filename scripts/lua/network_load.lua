ifname = _GET["if"]
if(ifname == nil) then
   ifname = "any"
end
interface.find(ifname)
ifstats = interface.getStats()

print('{ "packets": '.. ifstats.stats_packets .. ', "bytes": ' .. ifstats.stats_bytes .. ', "num_flows": '.. ifstats.stats_flows .. ', "num_hosts": ' .. ifstats.stats_hosts .. ' }')
