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

interface.find(ifname)
ifstats = interface.getStats()

interface_name = purifyInterfaceName(ifstats.name)

rrdname = dirs.workingdir .. "/" .. interface_name .. "/rrd/bytes.rrd"

_ifname = tostring(interface.name2id(ifname))
url= '/lua/if_stats.lua?ifname=' .. _ifname

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

if(ifstats.type ~= "zmq") then
   if(page == "packets") then
      print("<li class=\"active\"><a href=\"#\">Packets</a></li>\n")
   else
      print("<li><a href=\""..url.."&page=packets\">Packets</a></li>")
   end
end

if(page == "ndpi") then
  print("<li class=\"active\"><a href=\"#\">Protocols</a></li>\n")
else
   print("<li><a href=\""..url.."&page=ndpi\">Protocols</a></li>")
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

if((page == "overview") or (page == nil)) then
   print("<table class=\"table table-bordered\">\n")
   print("<tr><th width=250>Name</th><td>" .. ifstats.name .. "</td></tr>\n")
   print("<tr><th>Family</th><td>" .. ifstats.type .. "</td></tr>\n")
   print("<tr><th>Bytes</th><td><div id=if_bytes>" .. bytesToSize(ifstats.stats_bytes) .. "</div>");
   print [[
	 <p>
	 <small>
	 <div class="alert alert-info">
	    <b>NOTE</b>: In ethernet networks, each packet has an <A HREF=https://en.wikipedia.org/wiki/Ethernet_frame>overhead of 24 bytes</A> [preamble (7 bytes), start of freme (1 byte), CRC (4 bytes), and <A HREF=http://en.wikipedia.org/wiki/Interframe_gap>IFG</A> (12 bytes)]. Such overhead needs to be accounted to the interface traffic, but it is not added to the traffic being exchanged between IP addresses. This is because such data contributes to interface load, but it cannot be accounted in the traffic being exchanged by hosts, and thus expect little discrepancies between host and interface traffic values.
         </div></small>
	 </td></tr>
   ]]

if(ifstats.type ~= "zmq") then
   label = "Packets"
else
   label = "Flows"
end
   print("<tr><th>Received "..label.."</th><td><span id=if_pkts>" .. formatValue(ifstats.stats_packets) .. " " .. label .. "</span> <span id=pkts_trend></span></td></tr>\n")

   print("<tr><th>Dropped "..label.."</th><td><span id=if_drops>")
   
   if(ifstats.stats_drops > 0) then print('<span class="label label-important">') end
   print(formatValue(ifstats.stats_drops).. " " .. label)
   
   if((ifstats.stats_packets+ifstats.stats_drops) > 0) then
      local pctg = round((ifstats.stats_drops*100)/(ifstats.stats_packets+ifstats.stats_drops), 2)   
      if(pctg > 0) then print(" [ " .. pctg .. " % ] ") end
   end
   
   if(ifstats.stats_drops > 0) then print('</span>') end
   print("</span>  <span id=drops_trend></span></td></tr>\n")
   
   print("</table>\n")
elseif((page == "packets")) then
      print [[

      <table class="table table-bordered table-striped">
      	<tr><th class="text-center">Size Distribution</th><td colspan=5><div class="pie-chart" id="sizeDistro"></div></td></tr>
      </table>

        <script type='text/javascript'>
	       window.onload=function() {
		   var refresh = 3000 /* ms */;
		   do_pie("#sizeDistro", '/lua/if_pkt_distro.lua', { type: "size", ifname: "]] print(_ifname.."\"")
	print [[
	         }, "", refresh);
		}

	    </script><p>
	]]
elseif(page == "ndpi") then
      print [[

      <table class="table table-bordered table-striped"> 
     	<tr><th class="text-center">Protocol Overview</th><td colspan=5><div class="pie-chart" id="topApplicationProtocols"></div></td></tr>
	</div>

        <script type='text/javascript'>
	       window.onload=function() {
		   var refresh = 3000 /* ms */;
		   do_pie("#topApplicationProtocols", '/lua/iface_ndpi_stats.lua', { mode: "sinceStartup", ifname: "]] print(_ifname) print [[" }, "", refresh);
		}

	    </script><p>
	]]

     print("<tr><th>Application Protocol</th><th>Total (since ntopng startup)</th><th colspan=2>Percentage</th></tr>\n")

      total = ifstats["stats_bytes"]

      vals = {}
      for k in pairs(ifstats["ndpi"]) do
	 vals[k] = k
      end
      table.sort(vals)

      for _k in pairsByKeys(vals , desc) do
	 k = vals[_k]
	 print("<tr><th>"..k)
	 t = ifstats["ndpi"][k]["bytes.sent"]+ifstats["ndpi"][k]["bytes.rcvd"]
	 print("</th><td class=\"text-right\">" .. bytesToSize(t).. "</td>")
         print("<td>")
         percentageBar(total, ifstats["ndpi"][k]["bytes.rcvd"], k)
         print("</td>\n")
	 print("<td class=\"text-right\">" .. round((t * 100)/total, 2).. " %</td></tr>\n")
      end

      print("</table>\n")
else
   drawRRD(interface_name, nil, "bytes.rrd", _GET["graph_zoom"], url.."&page=historical", 0, _GET["epoch"], "/lua/top_talkers.lua")
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
			    data: { ifname: "]] print(tostring(interface.name2id(ifstats.name))) print [[" },
			    success: function(content) {
				var rsp = jQuery.parseJSON(content);
				$('#if_bytes').html(bytesToVolume(rsp.bytes));
				$('#if_pkts').html(addCommas(rsp.packets)+"]]


if(ifstats.type == "zmq") then print(" Flows\");") else print(" Pkts\");") end
print [[
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
				drops = drops + addCommas(rsp.drops)+" ]]

if(ifstats.type == "zmq") then print("Flows") else print("Pkts") end
print [[";

				if(pctg > 0)      { drops = drops + " [ "+pctg+" % ]"; }
				if(rsp.drops > 0) { drops = drops + '</span>';         }
				$('#if_drops').html(drops);
			     }
		           });
			 }, 3000)

</script>

]]
