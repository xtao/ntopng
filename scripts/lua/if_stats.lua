--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "graph_utils"

sendHTTPHeader('text/html')

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")

active_page = "if_stats"
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

page = _GET["page"]

ifname = _GET["interface"]
if(ifname == nil) then
   ifname = "any"
end

rrdname = dirs.workingdir .. "/rrd/interface.any/bytes.rrd"

url= '/lua/if_stats.lua?interface=' .. ifname

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
   print("<tr><th width=250>Name</th><td>" .. ifstats.name .. "</td></tr>\n")
   print("<tr><th>Bytes</th><td><div id=if_bytes>" .. bytesToSize(ifstats.stats_bytes) .. "</div></td></tr>\n")
   print("<tr><th>Received Packets</th><td><div id=if_pkts>" .. formatPackets(ifstats.stats_packets) .. "</div></td></tr>\n")
   print("<tr><th>Dropped Packets</th><td><div id=if_drops>")

   if(ifstats.stats_drops > 0) then print('<span class="label label-important">') end
   print(formatPackets(ifstats.stats_drops))

   if((ifstats.stats_packets+ifstats.stats_drops) > 0) then
      local pctg = round((ifstats.stats_drops*100)/(ifstats.stats_packets+ifstats.stats_drops), 2)   
      print(" [ " .. pctg .. " % ] ")
   end

   if(ifstats.stats_drops > 0) then print('</span>') end
   print("</div></td></tr>\n")
   print("</table>\n")
else
   drawRRD('interface.any', "bytes.rrd", _GET["graph_zoom"], url.."&page=historical", 0, _GET["epoch"], "/lua/top_talkers.lua")
end


dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")

print [[
<script>

setInterval(function() {
		  $.ajax({
			    type: 'GET',
			    url: '/lua/network_load.lua',
			    data: { if: "]] print(ifstats.name) print [[" },
			    success: function(content) {
				var rsp = jQuery.parseJSON(content);
				$('#if_bytes').html(bytesToVolume(rsp.bytes));
				$('#if_pkts').html(addCommas(rsp.packets)+" Pkts");
				var pctg =  ((rsp.drops*100)/(rsp.packets+rsp.drops)).toFixed(0);
				$('#if_drops').html(addCommas(rsp.drops)+" Pkts [ "+pctg+" % ]");


			     }
		           });
			 }, 3000)

</script>

]]
