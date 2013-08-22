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

rrdname = dirs.workingdir .. "/" .. ifname .. "/rrd/bytes.rrd"

url= '/lua/if_stats.lua?ifname=' .. ifname

print [[
            <div class="navbar">
              <div class="navbar-inner">
	      <ul class="nav">
]]

-- print("<li><a href=\"#\">Interface " .. ifname .."</a></li>\n")

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
   ]]

interface.find(ifname)
ifstats = interface.getStats()

if((page == "overview") or (page == nil)) then
   print("<table class=\"table table-bordered\">\n")
   print("<tr><th width=250>Name</th><td>" .. ifstats.name .. "</td></tr>\n")
   print("<tr><th>Bytes</th><td><div id=if_bytes>" .. bytesToSize(ifstats.stats_bytes) .. "</div>");
   print [[
	 <p>
	 <small>
	 <div class="alert alert-info">
	    <b>NOTE</b>: In ethernet networks, each packet has an <A HREF=https://en.wikipedia.org/wiki/Ethernet_frame>overhead of 24 bytes</A> [preamble (7 bytes), start of freme (1 byte), CRC (4 bytes), and <A HREF=http://en.wikipedia.org/wiki/Interframe_gap>IFG</A> (12 bytes)]. Such overhead needs to be accounted to the interface traffic, but it is not added to the traffic being exchanged between IP addresses. This is because such data contributes to interface load, but it cannot be accounted in the traffic being exchanged by hosts, and thus expect little discrepancies between host and interface traffic values.
         </div></small>
	 </td></tr>
   ]]

   print("<tr><th>Received Packets</th><td><span id=if_pkts>" .. formatPackets(ifstats.stats_packets) .. "</span> <span id=pkts_trend></span></td></tr>\n")
   print("<tr><th>Dropped Packets</th><td><span id=if_drops>")

   if(ifstats.stats_drops > 0) then print('<span class="label label-important">') end
   print(formatPackets(ifstats.stats_drops))

   if((ifstats.stats_packets+ifstats.stats_drops) > 0) then
      local pctg = round((ifstats.stats_drops*100)/(ifstats.stats_packets+ifstats.stats_drops), 2)   
      if(pctg > 0) then print(" [ " .. pctg .. " % ] ") end
   end

   if(ifstats.stats_drops > 0) then print('</span>') end
   print("</span>  <span id=drops_trend></span></td></tr>\n")
   print("</table>\n")
else
   drawRRD(ifname, nil, "bytes.rrd", _GET["graph_zoom"], url.."&page=historical", 0, _GET["epoch"], "/lua/top_talkers.lua")
end


dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")

print("<script>\n")
print("var last_pkts  = " .. ifstats.stats_packets .. ";\n")
print("var last_drops = " .. ifstats.stats_drops .. ";\n")

print [[
setInterval(function() {
		  $.ajax({
			    type: 'GET',
			    url: '/lua/network_load.lua',
			    data: { ifname: "]] print(ifstats.name) print [[" },
			    success: function(content) {
				var rsp = jQuery.parseJSON(content);
				$('#if_bytes').html(bytesToVolume(rsp.bytes));
				$('#if_pkts').html(addCommas(rsp.packets)+" Pkts");
				var pctg = 0;
				var drops = "";

				if(last_pkts == rsp.packets) {
				   $('#pkts_trend').html("<i class=icon-minus></i>");
				} else {
				   $('#pkts_trend').html("<i class=icon-arrow-up></i>");
				}
				if(last_drops == rsp.drops) {
				   $('#drops_trend').html("<i class=icon-minus></i>");
				} else {
				   $('#drops_trend').html("<i class=icon-arrow-up></i>");
				}
				last_pkts = rsp.packets;
				last_drops == rsp.drops;

				if((rsp.packets+rsp.drops) > 0)	{ pctg = ((rsp.drops*100)/(rsp.packets+rsp.drops)).toFixed(2); }
				if(rsp.drops > 0) { drops = '<span class="label label-important">'; }
				drops = drops + addCommas(rsp.drops)+" Pkts ";
				if(pctg > 0)      { drops = drops + " [ "+pctg+" % ]"; }
				if(rsp.drops > 0) { drops = drops + '</span>';         }
				$('#if_drops').html(drops);
			     }
		           });
			 }, 3000)

</script>

]]
