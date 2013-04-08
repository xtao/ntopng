ifname = _GET["if"]
interface.find("any")
ifstats = interface.getLoadStats()

print('{ "packets": '.. ifstats.packets .. ', "bytes": ' .. ifstats.bytes .. ' }')
