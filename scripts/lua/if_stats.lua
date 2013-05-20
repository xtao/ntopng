package.path = "./scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"
require "graph_utils"

ntop.dumpFile("./httpdocs/inc/header.inc")
dofile("./scripts/lua/menu.lua")

page = _GET["page"]

ifname = _GET["interface"]
if(ifname == nil) then
   ifname = "any"
end

rrdname = ntop.getDataDir() .. "/rrd/interface.any/bytes.rrd"

url= '/if_stats.lua?interface=' .. ifname

print [[
<div class="bs-docs-example">
            <div class="navbar">
              <div class="navbar-inner">
<ul class="nav">
]]



if((page == "overview") or (page == nil)) then
  print("<li class=\"active\"><a href=\"#\">Overview</a></li>\n")
else
  print("<li><a href=\""..url.."&page=overview\">Overview</a></li>")
end

if(ntop.exists(rrdname)) then
if(page == "historical") then
  print("<li class=\"active\"><a href=\"#\">Historical Activity</a></li>\n")
else
  print("<li><a href=\""..url.."&page=historical\">Historical Activity</a></li>")
end
end

print [[
</ul>
</div>
</div>
</div>
   ]]


if((page == "overview") or (page == nil)) then
   interface.find(ifname)
   ifstats = interface.getStats()

   print("<table class=\"table table-bordered\">\n")
   print("<tr><th>Name</th><td>" .. ifstats.name .. "</td></tr>\n")
   print("<tr><th>Bytes</th><td>" .. bytesToSize(ifstats.stats_bytes) .. "</td></tr>\n")
   print("<tr><th>Packets</th><td>" .. ifstats.stats_packets .. "</td></tr>\n")
   print("</table>\n")
else
drawRRD('interface.any', "bytes.rrd", _GET["graph_zoom"], url.."&page=historical", 0)
end

dofile "./scripts/lua/footer.inc.lua"
