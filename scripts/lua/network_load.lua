ifname = _GET["if"]
interface.find("any")
ifstats = interface.getStats()

print('{ "packets": '.. ifstats.stats_packets .. ', "bytes": ' .. ifstats.stats_bytes .. ', "num_flows": '.. ifstats.stats_flows .. ', "num_hosts": ' .. ifstats.stats_hosts .. ' }')
