--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.workingdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "graph_utils"

sendHTTPHeader('text/html')

ntop.dumpFile(dir.workingdir .. "/httpdocs/inc/header.inc")

active_page = "hosts"
dofile(dirs.workingdir .. "/scripts/lua/inc/menu.lua")

page = _GET["page"]

ifname = _GET["interface"]
if(ifname == nil) then
   ifname = "any"
end

host_ip = _GET["host"]

if(host_ip == nil) then
   print("<div class=\"alert alert-error\"><img src=/img/warning.png> Host parameter is missing (internal error ?)</div>")
   return
end

interface.find(ifname)
host = interface.getHostInfo(host_ip)

if(host == nil) then
   print("<div class=\"alert alert-error\"><img src=/img/warning.png> Host ".. host_ip .. " cannot be found (expired ?)</div>")
   return
else
   if(host["ip"] ~= nil) then
      host_ip = host["ip"]
   end

   rrdname = ntop.getDataDir() .. "/rrd/" .. host_ip .. "/bytes.rrd"
print [[
<div class="bs-docs-example">
            <div class="navbar">
              <div class="navbar-inner">
<ul class="nav">
]]

url="/lua/host_details.lua?host="..host_ip

print("<li><a href=\"#\">Host: "..host_ip.." </a></li>\n")

if((page == "overview") or (page == nil)) then
  print("<li class=\"active\"><a href=\"#\">Overview</a></li>\n")
else
  print("<li><a href=\""..url.."&page=overview\">Overview</a></li>")
end

if(page == "traffic") then
   print("<li class=\"active\"><a href=\"#\">Traffic</a></li>\n")
else
   if(host["ip"] ~= nil) then
      print("<li><a href=\""..url.."&page=traffic\">Traffic</a></li>")
   end
end

if(page == "ndpi") then
  print("<li class=\"active\"><a href=\"#\">Protocols</a></li>\n")
else
   if(host["ip"] ~= nil) then
      print("<li><a href=\""..url.."&page=ndpi\">Protocols</a></li>")
   end
end

if(page == "flows") then
  print("<li class=\"active\"><a href=\"#\">Active Flows</a></li>\n")
else
   if(host["ip"] ~= nil) then
      print("<li><a href=\""..url.."&page=flows\">Active Flows</a></li>")
   end
end

if(page == "talkers") then
  print("<li class=\"active\"><a href=\"#\">Talkers</a></li>\n")
else
   if(host["ip"] ~= nil) then
      print("<li><a href=\""..url.."&page=talkers\">Talkers</a></li>")
   end
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
   print("<table class=\"table table-bordered\">\n")


   if(host["ip"] ~= nil) then
      -- print("<tr><th>(Router) MAC Address</th><td><A HREF=\"host_details.lua?interface=" .. ifname .. "&host=" .. host["mac"].. "\">" .. host["mac"].."</A></td></tr>\n")
      print("<tr><th>(Router) MAC Address</th><td>" .. host["mac"].. "</td></tr>\n")
      print("<tr><th>IP Address</th><td>" .. host["ip"])
   else
      print("<tr><th>MAC Address</th><td>" .. host["mac"].. "</td></tr>\n")
   end

   if((host["city"] ~= "") or (host["country"] ~= "")) then
      print(" [ " .. host["city"] .. " <img src=\"/img/blank.gif\" class=\"flag flag-".. string.lower(host["country"]) .."\"> ]")
   end
   
   print("</td></tr>\n")

   if(host["vlan"] > 0) then print("<tr><th>VLAN Id</th><td>"..host["vlan"].."</td></tr>\n") end
   if(host["asn"] > 0) then print("<tr><th>ASN</th><td>"..host["asn"].." [ " .. host.asname .. " ] </td></tr>\n") end

   if(host["category"] ~= "") then print("<tr><th>Category</th><td>"..getCategory(host["category"]).."</td></tr>\n") end

   if(host["ip"] ~= nil) then
      print("<tr><th>Name</th><td><A HREF=\"http://" .. host["name"] .. "\">".. host["name"] .. "</A> ")

   if(host["localhost"] == true) then print('<span class="label label-success">Local</span>') else print('<span class="label">Remote</span>') end
   print("</td></tr>\n")
end

   print("<tr><th>First Seen</th><td>" .. os.date("%x %X", host["seen.first"]) ..  " [" .. secondsToTime(os.time()-host["seen.first"]) .. " ago]" .. "</td></tr>\n")
   print("<tr><th>Last Seen</th><td>" .. os.date("%x %X", host["seen.last"]) .. " [" .. secondsToTime(os.time()-host["seen.last"]) .. " ago]" .. "</td></tr>\n")

   print("<tr><th>Sent vs Received Traffic Breakdown</th><td>")
   breakdownBar(host["bytes.sent"], "Sent", host["bytes.rcvd"], "Rcvd")
   print("</td></tr>\n")

   print("<tr><th>Traffic Sent</th><td>" .. formatPackets(host["pkts.sent"]) .. " / ".. bytesToSize(host["bytes.sent"]) .. "</td></tr>\n")
   print("<tr><th>Traffic Received</th><td>" .. formatPackets(host["pkts.rcvd"]) .. " / ".. bytesToSize(host["bytes.rcvd"]) .. "</td></tr>\n")
   print("</table>\n")

   elseif((page == "traffic")) then
     total = 0
     for id, _ in ipairs(l4_keys) do
	k = l4_keys[id][2]
	total = total + host[k..".bytes.sent"] + host[k..".bytes.rcvd"]
     end

     if(total == 0) then
	print("<div class=\"alert alert-error\"><img src=/img/warning.png> No traffic has been observed for the specified host</div>")
     else
      print [[

      <table class="table table-bordered table-striped">
      	<tr><th class="text-center">Protocol Overview</th><td colspan=5><div class="pie-chart" id="topApplicationProtocols"></div></td></tr>
	</div>

        <script type='text/javascript'>
	       window.onload=function() {
				   var refresh = 3000 /* ms */;
				   do_pie("#topApplicationProtocols", '/lua/host_l4_stats.lua', { if: "any", host: ]]
	print("\""..host_ip.."\"")
	print [[ }, "", refresh);
				}

	    </script><p>
	]]

     print("<tr><th>Protocol</th><th>Sent</th><th>Received</th><th>Breakdown</th><th colspan=2>Total</th></tr>\n")
     

     for id, _ in ipairs(l4_keys) do
	label = l4_keys[id][1]
	k = l4_keys[id][2]
	sent = host[k..".bytes.sent"]
	rcvd = host[k..".bytes.rcvd"]

	if((sent > 0) or (rcvd > 0)) then
	   -- if(true) then
	    print("<tr><th>")
	    print("<A HREF=\"/lua/host_details.lua?host=" .. host_ip .. "&page=historical&rrd_file=".. k ..".rrd\">".. label .."</A>")
	    t = sent+rcvd
	    print("</th><td>" .. bytesToSize(sent) .. "</td><td>" .. bytesToSize(rcvd) .. "</td><td>")
	    breakdownBar(sent, "Sent", rcvd, "Rcvd")
	    print("</td><td>" .. bytesToSize(t).. "</td><td>" .. round((t * 100)/total, 2).. " %</td></tr>\n")
	 end
      end
      print("</table></tr>\n")
      print("</table>\n")
   end

   elseif((page == "ndpi")) then
   if(host["ndpi"] ~= nil) then
      print [[

      <table class="table table-bordered table-striped">
      	<tr><th class="text-center">Protocol Overview</th><td colspan=5><div class="pie-chart" id="topApplicationProtocols"></div></td></tr>
	</div>

        <script type='text/javascript'>
	       window.onload=function() {
				   var refresh = 3000 /* ms */;
				   do_pie("#topApplicationProtocols", '/lua/iface_ndpi_stats.lua', { if: "any", host: ]]
	print("\""..host_ip.."\"")
	print [[ }, "", refresh);
				}

	    </script><p>
	]]

      print("<tr><th>Application Protocol</th><th>Sent</th><th>Received</th><th>Breakdown</th><th colspan=2>Total</th></tr>\n")

      total = host["bytes.sent"]+host["bytes.rcvd"]

      vals = {}
      for k in pairs(host["ndpi"]) do
	 vals[k] = k
	 -- print(k)
      end
      table.sort(vals)

      print("<tr><th>Total</th><td  class=\"text-right\">" .. bytesToSize(host["bytes.sent"]) .. "</td><td  class=\"text-right\">" .. bytesToSize(host["bytes.rcvd"]) .. "</td>")

      print("<td>")
      breakdownBar(host["bytes.sent"], "Sent", host["bytes.rcvd"], "Rcvd")
      print("</td>\n")

      print("<td colspan=2>" ..  bytesToSize(total).. "</td></tr>\n")

      for _k in pairsByKeys(vals , desc) do
	 k = vals[_k]
	 print("<tr><th>")
	 print("<A HREF=\"/lua/host_details.lua?host=" .. host_ip .. "&page=historical&rrd_file=".. k ..".rrd\">"..k.."</A>")
	 t = host["ndpi"][k]["bytes.sent"]+host["ndpi"][k]["bytes.rcvd"]
	 print("</th><td>" .. bytesToSize(host["ndpi"][k]["bytes.sent"]) .. "</td><td>" .. bytesToSize(host["ndpi"][k]["bytes.rcvd"]) .. "</td>")

	 print("<td>")
	 breakdownBar(host["ndpi"][k]["bytes.sent"], "Sent", host["ndpi"][k]["bytes.rcvd"], "Rcvd")
	 print("</td>\n")

	 print("<td>" .. bytesToSize(t).. "</td><td>" .. round((t * 100)/total, 2).. " %</td></tr>\n")
      end

      print("</table>\n")
   end

   elseif(page == "flows") then

print [[
      <div id="table-hosts"></div>
	 <script>
	 $("#table-hosts").datatable({
				  ]]
				  print("url: \"/lua/get_flows_data.lua?host=" .. host_ip.."\",\n")


print [[
	       showPagination: true,
	       title: "Active Flows",
	        columns: [
			     {
			     title: "Info",
				 field: "column_key",
	 	             css: {
			        textAlign: 'center'
			     }
				 },
			     {
			     title: "Application",
				 field: "column_ndpi",
				 sortable: true,
	 	             css: {
			        textAlign: 'center'
			     }
				 },
			     {
			     title: "L4 Proto",
				 field: "column_proto_l4",
				 sortable: true,
	 	             css: {
			        textAlign: 'center'
			     }
				 },
			     {
			     title: "Client",
				 field: "column_client",
				 sortable: true,
				 },
			     {
			     title: "Server",
				 field: "column_server",
				 sortable: true,
				 },
			     {
			     title: "Duration",
				 field: "column_duration",
				 sortable: true,
	 	             css: {
			        textAlign: 'right'
			       }
			       },
			     {
			     title: "Bytes",
				 field: "column_bytes",
				 sortable: true,
	 	             css: {
			        textAlign: 'right'
			     }

				 }
			     ]
	       });
       </script>

   ]]
elseif(page == "talkers") then
print("<center>")
dofile(dirs.workingdir .. "/scripts/lua/inc/sankey.lua")
print("</center>")
elseif(page == "historical") then
if(_GET["rrd_file"] == nil) then
   rrdfile = "bytes.rrd"
else
   rrdfile=_GET["rrd_file"]
end

drawRRD(host_ip, rrdfile, _GET["graph_zoom"], '/lua/host_details.lua?host='..host_ip..'&page=historical', 1, _GET["epoch"])
else
   print(page)
end
end
dofile dirs.workingdir .. "/scripts/lua/inc/footer.lua"
