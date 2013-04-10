ifname = _GET["if"]
interface.find("any")
ifstats = interface.getStats()

print('{ "packets": '.. ifstats.packets .. ', "bytes": ' .. ifstats.bytes .. ', "num_flows": '.. ifstats.num_flows .. ', "num_hosts": ' .. ifstats.num_hosts .. ' }')
