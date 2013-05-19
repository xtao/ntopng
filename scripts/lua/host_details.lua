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

rrdname = ntop.getDataDir() .. "/rrd/" .. _GET["host"] .. "/bytes.rrd"

host_ip = _GET["host"]

interface.find(ifname)
host = interface.getHostInfo(host_ip)



if(host == nil) then
   print("<div class=\"alert alert-error\"><img src=/img/warning.png> Host ".. host_ip .. " cannot be found (expired ?)</div>")
else

   host_ip = host["ip"]

print [[
<div class="bs-docs-example">
            <div class="navbar">
              <div class="navbar-inner">
<ul class="nav">
]]

url="/host_details.lua?host="..host_ip

print("<li><a href=\"#\">Host: "..host_ip.." </a></li>\n")

if((page == "overview") or (page == nil)) then
  print("<li class=\"active\"><a href=\"#\">Overview</a></li>\n")
else
  print("<li><a href=\""..url.."&page=overview\">Overview</a></li>")
end

if(page == "ndpi") then
  print("<li class=\"active\"><a href=\"#\">L7 Protocols</a></li>\n")
else
  print("<li><a href=\""..url.."&page=ndpi\">L7 Protocols</a></li>")
end

if(page == "flows") then
  print("<li class=\"active\"><a href=\"#\">Active Flows</a></li>\n")
else
  print("<li><a href=\""..url.."&page=flows\">Active Flows</a></li>")
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
   print("<tr><th>(Router) MAC Address</th><td>" .. host["mac"].. "</td></tr>\n")
   print("<tr><th>IP Address</th><td>" .. host["ip"] .. "</td></tr>\n")
   print("<tr><th>Name</th><td>" .. host["name"] .. "</td></tr>\n")
   print("<tr><th>First Seen</th><td>" .. os.date("%x %X", host["seen.first"]) ..  " [" .. secondsToTime(os.time()-host["seen.first"]) .. " ago]" .. "</td></tr>\n")
   print("<tr><th>Last Seen</th><td>" .. os.date("%x %X", host["seen.last"]) .. " [" .. secondsToTime(os.time()-host["seen.last"]) .. " ago]" .. "</td></tr>\n")

   print("</table>\n")
   elseif((page == "ndpi")) then

   if(host["ndpi"] ~= nil) then
      print [[

      <table class="table table-bordered table-striped">
      	<tr><th class="text-center">Protocol Overview</th><td colspan=4><div class="pie-chart" id="topApplicationProtocols"></div></td></tr>
	</div>

        <script type='text/javascript'>
	       window.onload=function() {
				   var refresh = 3000 /* ms */;
				   do_pie("#topApplicationProtocols", '/iface_ndpi_stats.lua', { if: "any", host: ]]
	print("\""..host_ip.."\"")
	print [[ }, "", refresh);
				}

	    </script><p>
	]]

      print("<tr><th>Application Protocol</th><th>Sent</th><th>Received</th><th colspan=2>Total</th></tr>\n")

      total = host["bytes.sent"]+host["bytes.rcvd"]

      vals = {}
      for k in pairs(host["ndpi"]) do
	 vals[k] = k
	 -- print(k)
      end
      table.sort(vals)

      print("<tr><th>Total</th><td  class=\"text-right\">" .. bytesToSize(host["bytes.sent"]) .. "</td><td  class=\"text-right\">" .. bytesToSize(host["bytes.rcvd"]) .. "</td><td colspan=2>" ..  bytesToSize(total).. "</td></tr>\n")

      for _k in pairsByKeys(vals , desc) do
	 k = vals[_k]
	 print("<tr><th>")
	 print("<A HREF=\"/host_details.lua?host=" .. host_ip .. "&page=historical&rrd_file=".. k ..".rrd\">"..k.."</A>")
	 t = host["ndpi"][k]["bytes.sent"]+host["ndpi"][k]["bytes.rcvd"]
	 print("</th><td>" .. bytesToSize(host["ndpi"][k]["bytes.sent"]) .. "</td><td>" .. bytesToSize(host["ndpi"][k]["bytes.rcvd"]) .. "</td><td>" .. bytesToSize(t).. "</td><td>" .. round((t * 100)/total, 2).. " %</td></tr>\n")
      end

      print("</table>\n")
   end

   elseif(page == "flows") then

print [[
      <div id="table-hosts"></div>
	 <script>
	 $("#table-hosts").datatable({
				  ]]
				  print("url: \"/get_flows_data.lua?host=" .. host_ip.."\",\n")


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
elseif(page == "historical") then
if(_GET["rrd_file"] == nil) then
   rrdfile = "bytes.rrd"
else
   rrdfile=_GET["rrd_file"]
end

drawRRD(host_ip, rrdfile, _GET["graph_zoom"], '/host_details.lua?host='..host_ip..'&page=historical')
else
   print(page)
end
end
dofile "./scripts/lua/footer.inc.lua"
