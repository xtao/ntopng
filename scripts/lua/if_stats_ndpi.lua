--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "graph_utils"


sendHTTPHeader('text/html; charset=iso-8859-1')

interface.find(ifname)
is_historical = interface.isHistoricalInterface(interface.name2id(ifname))
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
  if(ntop.exists(fname) and not is_historical) then
    print("<A HREF=\""..ntop.getHttpPrefix().."/lua/if_stats.lua?if_name=" .. ifname .. "&page=historical&rrd_file=".. k ..".rrd\">".. k .."</A>")
  else
    print(k)
  end

  print(" <A HREF="..ntop.getHttpPrefix().."/lua/flows_stats.lua?application="..k.."><i class=\"fa fa-search-plus\"></i></A></th>")

  t = ifstats["ndpi"][k]["bytes.sent"]+ifstats["ndpi"][k]["bytes.rcvd"]

  print("<td class=\"text-right\" style=\"width: 20%;\">" .. bytesToSize(t).. "</td>")
  print("<td ><span style=\"width: 60%; float: left;\">")
  percentageBar(total, ifstats["ndpi"][k]["bytes.rcvd"], "") -- k
  --         print("</td>\n")
  print("</span><span style=\"width: 40%; margin-left: 15px;\" >" ..round((t * 100)/total, 2).. " %</span></td></tr>\n")
end

