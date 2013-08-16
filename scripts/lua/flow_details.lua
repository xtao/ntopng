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
      print("<tr><th width=30%>VLAN ID</th><td colspan=2>" .. flow["vlan"].. "</td></tr>\n")
   end
   print("<tr><th width=30%>Client</th><td colspan=2><A HREF=\"/lua/host_details.lua?host=" .. flow["cli.ip"] .. "\">")
   if(flow["cli.host"] ~= "") then print(flow["cli.host"]) else print(flow["cli.ip"]) end
   print("</A>:<A HREF=\"/lua/port_details.lua?port=" .. flow["cli.port"].. "\">" .. flow["cli.port"].. "</A></td></tr>\n")
   print("<tr><th width=30%>Server</th><td colspan=2><A HREF=\"/lua/host_details.lua?host=" .. flow["srv.ip"] .. "\">")
   if(flow["srv.host"] ~= "") then print(flow["srv.host"]) else print(flow["srv.ip"]) end
   print("</A>:<A HREF=\"/lua/port_details.lua?port=" .. flow["srv.port"].. "\">" .. flow["srv.port"].. "</A></td></tr>\n")

   if(flow["category"] ~= "") then
      print("<tr><th width=30%>Category</th><td colspan=2>" .. getCategory(flow["category"]) .. "</td></tr>\n")
   end
   print("<tr><th width=30%>Application Protocol</th><td colspan=2>" .. flow["proto.ndpi"] .. "</td></tr>\n")
   print("<tr><th width=30%>First Seen</th><td colspan=2><div id=first_seen>" .. formatEpoch(flow["seen.first"]) ..  " [" .. secondsToTime(os.time()-flow["seen.first"]) .. " ago]" .. "</div></td></tr>\n")
   print("<tr><th width=30%>Last Seen</th><td colspan=2><div id=last_seen>" .. formatEpoch(flow["seen.last"]) .. " [" .. secondsToTime(os.time()-flow["seen.last"]) .. " ago]" .. "</div></td></tr>\n")

   print("<tr><th width=30%>Total Traffic Volume</th><td colspan=2><div id=volume>" .. bytesToSize(flow["bytes"]) .. "</div></td></tr>\n")

   print("<tr><th width=30%>Client vs Server Traffic Breakdown</th><td colspan=2>")
   cli2srv = round((flow["cli2srv.bytes"] * 100) / flow["bytes"], 0)


   print('<div class="progress"><div class="bar bar-warning" style="width: ' .. cli2srv.. '%;">'.. flow["cli.ip"]..'</div><div class="bar bar-info" style="width: ' .. (100-cli2srv) .. '%;">' .. flow["srv.ip"] .. '</div></div>')
   print("</td></tr>\n")

   print("<tr><th width=30%>Client to Server Traffic</th><td colspan=2><div id=cli2srv>" .. formatPackets(flow["cli2srv.packets"]) .. " / ".. bytesToSize(flow["cli2srv.bytes"]) .. "</div></td></tr>\n")
   print("<tr><th width=30%>Server to Client Traffic</th><td colspan=2><div id=srv2cli>" .. formatPackets(flow["srv2cli.packets"]) .. " / ".. bytesToSize(flow["srv2cli.bytes"]) .. "</div></td></tr>\n")
   print("<tr><th width=30%>Actual Throughput</th><td width=20%>")
   
   print("<div id=throughput>" .. bitsToSize(8*flow["throughput"]) .. "</div>")
   print("</td><td><span id=thpt_load_chart>0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0</span>")

   print("</td></tr>\n")

   if(flow["tcp_flags"] > 0) then
      print("<tr><th width=30%>TCP Flags</th><td colspan=2>")

      if(hasbit(flow["tcp_flags"],0x01)) then print('<span class="label label-info">FIN</span> ')  end
      if(hasbit(flow["tcp_flags"],0x02)) then print('<span class="label label-info">SYN</span> ')  end
      if(hasbit(flow["tcp_flags"],0x04)) then print('<span class="label label-info">RST</span> ')  end
      if(hasbit(flow["tcp_flags"],0x08)) then print('<span class="label label-info">PUSH</span> ') end
      if(hasbit(flow["tcp_flags"],0x10)) then print('<span class="label label-info">ACK</span> ')  end
      if(hasbit(flow["tcp_flags"],0x20)) then print('<span class="label label-info">URG</span> ')  end

      print("</td></tr>\n")
   end

   local info, pos, err = json.decode(flow["moreinfo.json"], 1, nil)
   for key,value in pairs(info) do
      print("<tr><th width=30%>" .. getFlowKey(key) .. "</th><td colspan=2>" .. handleCustomFlowField(key, value) .. "</td></tr>\n")
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


var thptChart = $("#thpt_load_chart").peity("line", { width: 64 });

setInterval(function() {
	  $.ajax({
		    type: 'GET',
		    url: '/lua/flow_stats.lua',
		    data: { ifname: "]] print(ifname) print [[", flow_key: "]] print(flow_key) print [[" },
		    success: function(content) {
			var rsp = jQuery.parseJSON(content);
			$('#first_seen').html(rsp["seen.first"]);
			$('#last_seen').html(rsp["seen.last"]);
			$('#volume').html(bytesToVolume(rsp.bytes));
			$('#cli2srv').html(addCommas(rsp["cli2srv.packets"])+" Pkts / "+bytesToVolume(rsp["cli2srv.bytes"]));
			$('#srv2cli').html(addCommas(rsp["srv2cli.packets"])+" Pkts / "+bytesToVolume(rsp["srv2cli.bytes"]));
			$('#throughput').html(rsp.throughput);

			var values = thptChart.text().split(",");
			values.shift();
			values.push(rsp.throughput_raw);
			thptChart.text(values.join(",")).change();
		     }
	           });
		 }, 3000);

</script>
 ]]

dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
