--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "graph_utils"


sendHTTPHeader('text/html')

interface.find(ifname)
ifstats = interface.getStats()

total = ifstats["stats_bytes"]

vals = {}

for k in pairs(ifstats["ndpi"]) do
 vals[k] = k
end

table.sort(vals)

for _k in pairsByKeys(vals , desc) do
  k = vals[_k]
  print('<tr id="t_protocol_'..k..'">')
  print('<th style="width: 33%;">')

  fname = getRRDName(ifstats.id, nil, k..".rrd")
  if(ntop.exists(fname)) then
    print("<A HREF=\"/lua/if_stats.lua?if_name=" .. ifname .. "&page=historical&rrd_file=".. k ..".rrd\">".. k .."</A>")
  else
    print(k)
  end

  t = ifstats["ndpi"][k]["bytes.sent"]+ifstats["ndpi"][k]["bytes.rcvd"]
  print("</th><td class=\"text-right\" style=\"width: 20%;\">" .. bytesToSize(t).. "</td>")
  print("<td ><span style=\"width: 60%; float: left;\">")
  percentageBar(total, ifstats["ndpi"][k]["bytes.rcvd"], "") -- k
  --         print("</td>\n")
  print("</span><span style=\"width: 40%; margin-left: 15px;\" >" ..round((t * 100)/total, 2).. " %</span></td></tr>\n")
end

