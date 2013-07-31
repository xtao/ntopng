--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "flow_utils"
local json = require ("dkjson")

sendHTTPHeader('text/html')

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")

active_page = "flows"
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

print [[


<ul class="breadcrumb">
  <li><A HREF=/lua/flows_stats.lua>Flows</A> <span class="divider">/</span></li>
]]

print("<li>".._GET["label"].."</li></ul>")

ifname = _GET["if"]
if(ifname == nil) then	  
  ifname = "any"
end

flow_key = _GET["flow_key"]
if(flow_key == nil) then
   flow = nil
else
   interface.find(ifname)
   flow = interface.findFlowByKey(tonumber(flow_key))
end

if(flow == nil) then
   print("<div class=\"alert alert-error\"><img src=/img/warning.png> This flow cannot be found (expired ?)</div>")
else   
  
   print("<table class=\"table table-bordered\">\n")
   if(flow["vlan"] ~= 0) then
      print("<tr><th>VLAN ID</th><td>" .. flow["vlan"].. "</td></tr>\n")
   end
   print("<tr><th>Client</th><td><A HREF=\"/lua/host_details.lua?interface=" ..ifname .. "&host=" .. flow["src.ip"] .. "\">")
   if(flow["src.host"] ~= "") then print(flow["src.host"]) else print(flow["src.ip"]) end
   print("</A>:<A HREF=\"/lua/port_details.lua?interface=" ..ifname .. "&port=" .. flow["src.port"].. "\">" .. flow["src.port"].. "</A></td></tr>\n")
   print("<tr><th>Server</th><td><A HREF=\"/lua/host_details.lua?interface=" ..ifname .. "&host=" .. flow["dst.ip"] .. "\">")
   if(flow["dst.host"] ~= "") then print(flow["dst.host"]) else print(flow["dst.ip"]) end
   print("</A>:<A HREF=\"/lua/port_details.lua?interface=" ..ifname .. "&port=" .. flow["dst.port"].. "\">" .. flow["dst.port"].. "</A></td></tr>\n")

   if(flow["category"] ~= "") then 
      print("<tr><th>Category</th><td>" .. getCategory(flow["category"]) .. "</td></tr>\n")
   end				
   print("<tr><th>Application Protocol</th><td>" .. flow["proto.ndpi"] .. "</td></tr>\n")
   print("<tr><th>First Seen</th><td>" .. os.date("%x %X", flow["seen.first"]) ..  " [" .. secondsToTime(os.time()-flow["seen.first"]) .. " ago]" .. "</td></tr>\n")
   print("<tr><th>Last Seen</th><td><div id=last_seen>" .. os.date("%x %X", flow["seen.last"]) .. " [" .. secondsToTime(os.time()-flow["seen.last"]) .. " ago]" .. "</div></td></tr>\n")

   print("<tr><th>Total Traffic Volume</th><td><div id=volume>" .. bytesToSize(flow["bytes"]) .. "</div></td></tr>\n")

   print("<tr><th>Client vs Server Traffic Breakdown</th><td>")  
   cli2srv = round((flow["cli2srv.bytes"] * 100) / flow["bytes"], 0)


   print('<div class="progress"><div class="bar bar-warning" style="width: ' .. cli2srv.. '%;">'.. flow["src.ip"]..'</div><div class="bar bar-info" style="width: ' .. (100-cli2srv) .. '%;">' .. flow["dst.ip"] .. '</div></div>')
   print("</td></tr>\n")

   print("<tr><th>Client to Server Traffic</th><td><div id=cli2srv>" .. formatPackets(flow["cli2srv.packets"]) .. " / ".. bytesToSize(flow["cli2srv.bytes"]) .. "</div></td></tr>\n")
   print("<tr><th>Server to Client Traffic</th><td><div id=srv2cli>" .. formatPackets(flow["srv2cli.packets"]) .. " / ".. bytesToSize(flow["srv2cli.bytes"]) .. "</div></td></tr>\n")
   print("<tr><th>Actual Throughput</th><td><div id=throughput>" .. bitsToSize(8*flow["throughput"]) .. "</div></td></tr>\n")

   if(flow["tcp_flags"] > 0) then
      print("<tr><th>TCP Flags</th><td>")

      if(hasbit(flow["tcp_flags"],0x01)) then print('<span class="label label-info">FIN</span> ') end
      if(hasbit(flow["tcp_flags"],0x02)) then print('<span class="label label-info">SYN</span> ') end
      if(hasbit(flow["tcp_flags"],0x04)) then print('<span class="label label-info">RST</span> ') end
      if(hasbit(flow["tcp_flags"],0x08)) then print('<span class="label label-info">PUSH</span> ') end
      if(hasbit(flow["tcp_flags"],0x10)) then print('<span class="label label-info">ACK</span> ') end
      if(hasbit(flow["tcp_flags"],0x20)) then print('<span class="label label-info">URG</span> ') end
      
      print("</td></tr>\n")
   end

   local info, pos, err = json.decode(flow["moreinfo.json"], 1, nil)
   for key,value in pairs(info) do
      print("<tr><th>" .. getFlowKey(key) .. "</th><td>" .. handleCustomFlowField(key, value) .. "</td></tr>\n") 
   end

   print("</table>\n")
end

print [[
<script>
/*
      $(document).ready(function() {
	      $('.progress .bar').progressbar({ use_percentage: true, display_text: 1 });
   });
*/

setInterval(function() {
	  $.ajax({
		    type: 'GET',
		    url: '/lua/flow_stats.lua',
		    data: { if: "]] print(ifname) print [[", flow_key: "]] print(flow_key) print [[" },
		    success: function(content) {
			var rsp = jQuery.parseJSON(content);
			$('#last_seen').html(rsp["seen.last"]);
			$('#volume').html(bytesToVolume(rsp.bytes));
			$('#cli2srv').html(addCommas(rsp["cli2srv.packets"])+" Pkts / "+bytesToVolume(rsp["cli2srv.bytes"]));
			$('#srv2cli').html(addCommas(rsp["srv2cli.packets"])+" Pkts / "+bytesToVolume(rsp["srv2cli.bytes"]));
			$('#throughput').html(rsp.throughput);
		     }
	           });
		 }, 3000);

</script>
 ]]

dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
