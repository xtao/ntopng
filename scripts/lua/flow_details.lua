--
-- (C) 2013 - ntop.org
--

package.path = "./scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"
require "flow_utils"
local json = require ("dkjson")

sendHTTPHeader('text/html')

ntop.dumpFile("./httpdocs/inc/header.inc")

active_page = "flows"
dofile("./scripts/lua/inc/menu.lua")

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
   print("<tr><th>Last Seen</th><td>" .. os.date("%x %X", flow["seen.last"]) .. " [" .. secondsToTime(os.time()-flow["seen.last"]) .. " ago]" .. "</td></tr>\n")

   print("<tr><th>Total Traffic Volume</th><td>" .. bytesToSize(flow["bytes"]) .. "</td></tr>\n")

   print("<tr><th>Client vs Server Traffic Breakdown</th><td>")  
   cli2srv = round((flow["cli2srv.bytes"] * 100) / flow["bytes"], 0)


   print('<div class="progress"><div class="bar bar-warning" style="width: ' .. cli2srv.. '%;">'.. flow["src.ip"]..'</div><div class="bar bar-info" style="width: ' .. (100-cli2srv) .. '%;">' .. flow["dst.ip"] .. '</div></div>')
   print("</td></tr>\n")

   print("<tr><th>Client to Server Traffic</th><td>" .. formatPackets(flow["cli2srv.packets"]) .. " / ".. bytesToSize(flow["cli2srv.bytes"]) .. "</td></tr>\n")
   print("<tr><th>Server to Client Traffic</th><td>" .. formatPackets(flow["srv2cli.packets"]) .. " / ".. bytesToSize(flow["srv2cli.bytes"]) .. "</td></tr>\n")
  
   local info, pos, err = json.decode(flow["moreinfo.json"], 1, nil)
   for key,value in pairs(info) do
      print("<tr><th>" .. getFlowKey(key) .. "</th><td>" .. handleCustomFlowField(key, value) .. "</td></tr>\n") 
   end

   print("</table>\n")
end

print [[
<script>
      $(document).ready(function() {
	      $('.progress .bar').progressbar({ use_percentage: true, display_text: 1 });
   });
</script>
 ]]

dofile "./scripts/lua/inc/footer.lua"
