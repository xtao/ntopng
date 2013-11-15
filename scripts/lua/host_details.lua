--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "graph_utils"

page    = _GET["page"]
host_ip = _GET["host"]

active_page = "hosts"

if(host_ip == nil) then
   sendHTTPHeader('text/html')
   ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")
   dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")
   print("<div class=\"alert alert-error\"><img src=/img/warning.png> Host parameter is missing (internal error ?)</div>")
   return
end

_ifname = tostring(interface.name2id(ifname))
interface.find(ifname)

ip_elems = split(host_ip, " ");
host_ip = ip_elems[1]
host = nil
family = nil

--print(">>>") print(host_ip) print("<<<")

if(ip_elems[2] == nil) then
   host = interface.getHostInfo(host_ip)
   restoreFailed = false

   if((host == nil) and (_GET["mode"] == "restore")) then
      interface.restoreHost(host_ip)
      host = interface.getHostInfo(host_ip)
      restoreFailed = true
   end
else
   family = ip_elems[2]
end

if(host == nil) then
   -- We need to check if this is an aggregated host
   host = interface.getAggregatedHostInfo(host_ip)

   if(host == nil) then
      stats = interface.getIfNames()
      for id, name in pairs(stats) do
	 if(name == ifname) then
	    ifId = id
	    break
	 end
      end

      if(not(restoreFailed)) then json = ntop.getCache(host_ip.. "." .. ifId .. ".json") end
      sendHTTPHeader('text/html')
      ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")
      dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")
      print("<div class=\"alert alert-error\"><img src=/img/warning.png> Host ".. host_ip .. " cannot be found.")
      if((json ~= nil) and (json ~= "")) then
	 print('<p>Such host as been purged from memory due to inactivity. Click <A HREF="?host='..host_ip..'&mode=restore">here</A> to restore it from cache.\n')
      else
	 print('<p>Perhaps this host has been previously purged from memory or it has never been observed by this ntopng instance.</p>\n')
      end

      print("</div>")
      dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
   else
      print(ntop.httpRedirect("/lua/aggregated_host_details.lua?host="..host_ip))
   end
   return
else
   sendHTTPHeader('text/html')
   ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")
   dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

   if(host["ip"] ~= nil) then
      host_ip = host["ip"]
   end

   rrdname = dirs.workingdir .. "/" .. purifyInterfaceName(ifname) .. "/rrd/" .. host_ip .. "/bytes.rrd"
   --print(rrdname)
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

if(page == "packets") then
   print("<li class=\"active\"><a href=\"#\">Packets</a></li>\n")
else
   if(host["ip"] ~= nil) then
      print("<li><a href=\""..url.."&page=packets\">Packets</a></li>")
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
  print("<li class=\"active\"><a href=\"#\">Flows</a></li>\n")
else
   if(host["ip"] ~= nil) then
      print("<li><a href=\""..url.."&page=flows\">Flows</a></li>")
   end
end

if(page == "talkers") then
  print("<li class=\"active\"><a href=\"#\">Talkers</a></li>\n")
else
   if(host["ip"] ~= nil) then
      print("<li><a href=\""..url.."&page=talkers\">Talkers</a></li>")
   end
end

if(page == "geomap") then
  print("<li class=\"active\"><a href=\"#\">Geomap</a></li>\n")
else
   if((host["ip"] ~= nil) and (host["privatehost"] == false)) then
      print("<li><a href=\""..url.."&page=geomap\">Geomap</a></li>")
   end
end

cnum = 0
snum = 0
if(host.contacts ~= nil) then
   if(host["contacts"]["client"] ~= nil) then
      for k,v in pairs(host["contacts"]["client"]) do cnum = cnum + 1 end
   end

   if(host["contacts"]["server"] ~= nil) then
      for k,v in pairs(host["contacts"]["server"]) do snum = snum + 1 end
   end
end

num = cnum + snum

if(num > 0) then
   if(page == "contacts") then
      print("<li class=\"active\"><a href=\"#\">Contacts</a></li>\n")
   else
      print("<li><a href=\""..url.."&page=contacts\">Contacts</a></li>")
   end
end

if(interface.getNumAggregatedHosts() > 0) then
if(page == "aggregations") then
  print("<li class=\"active\"><a href=\"#\">Aggregations</a></li>\n")
else
  print("<li><a href=\""..url.."&page=aggregations\">Aggregations</a></li>")
end
end


if(ntop.exists(rrdname)) then
if(page == "historical") then
  print("<li class=\"active\"><a href=\"#\">Historical</a></li>\n")
else
  print("<li><a href=\""..url.."&page=historical\">Historical</a></li>")
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
      -- print("<tr><th>(Router) MAC Address</th><td><A HREF=\"host_details.lua?host=" .. host["mac"].. "\">" .. host["mac"].."</A></td></tr>\n")

      if(host["mac"] ~= "00:00:00:00:00:00") then
	 print("<tr><th width=35%>(Router) MAC Address</th><td>" .. host["mac"].. "</td></tr>\n")
      end
      print("<tr><th>IP Address</th><td>" .. host["ip"])
   else
      if(host["mac"] ~= nil) then
	 print("<tr><th>MAC Address</th><td>" .. host["mac"].. "</td></tr>\n")
      end
   end

   if((host["city"] ~= "") and (host["country"] ~= "")) then
      print(" [ " .. host["city"] .. " <img src=\"/img/blank.gif\" class=\"flag flag-".. string.lower(host["country"]) .."\"> ]")
   end

   print("</td></tr>\n")

   if((host["vlan"] ~= nil) and (host["vlan"] > 0)) then print("<tr><th>VLAN Id</th><td>"..host["vlan"].."</td></tr>\n") end
   if(host["os"] ~= "") then print("<tr><th>OS</th><td>" .. mapOS2Icon(host["os"]) .. " </td></tr>\n") end
   if((host["asn"] ~= nil) and (host["asn"] > 0)) then print("<tr><th>ASN</th><td>".. printASN(host["asn"], host.asname) .. " [ " .. host.asname .. " ] </td></tr>\n") end

   if((host["category"] ~= nil) and (host["category"] ~= "")) then 
      cat = getCategory(host["category"])
      
      if(cat ~= "") then
	 print("<tr><th>Category</th><td>".. cat .."</td></tr>\n") 
      end
   end

   if(host["ip"] ~= nil) then
      if(host["name"] == nil) then host["name"] = ntop.getResolvedAddress(host["ip"]) end
      print("<tr><th>Name</th><td><A HREF=\"http://" .. host["name"] .. "\"> <span id=name>")

      if(host["ip"] ==  host["name"]) then
	 print("<img border=0 src=/img/throbber.gif style=\"vertical-align:text-top;\"> ")
      end

      print(host["name"] .. "</span></A> <i class=\"fa fa-external-link fa-lg\"></i> ")

      if(host["localhost"] == true) then print('<span class="label label-success">Local</span>') else print('<span class="label">Remote</span>') end
      if(host["privatehost"] == true) then print(' <span class="label label-warn">Private IP</span>') end
   print("</td></tr>\n")
end

   print("<tr><th>First Seen</th><td><span id=first_seen>" .. formatEpoch(host["seen.first"]) ..  " [" .. secondsToTime(os.time()-host["seen.first"]) .. " ago]" .. "</span></td></tr>\n")
   print("<tr><th>Last Seen</th><td><span id=last_seen>" .. formatEpoch(host["seen.last"]) .. " [" .. secondsToTime(os.time()-host["seen.last"]) .. " ago]" .. "</span></td></tr>\n")

   if((host["bytes.sent"]+host["bytes.rcvd"]) > 0) then
      print("<tr><th>Sent vs Received Traffic Breakdown</th><td>")
      breakdownBar(host["bytes.sent"], "Sent", host["bytes.rcvd"], "Rcvd")
      print("</td></tr>\n")
   end

   print("<tr><th>Traffic Sent</th><td><span id=pkts_sent>" .. formatPackets(host["pkts.sent"]) .. "</span> / <span id=bytes_sent>".. bytesToSize(host["bytes.sent"]) .. "</span> <span id=sent_trend></span></td></tr>\n")
   print("<tr><th>Traffic Received</th><td><span id=pkts_rcvd>" .. formatPackets(host["pkts.rcvd"]) .. "</span> / <span id=bytes_rcvd>".. bytesToSize(host["bytes.rcvd"]) .. "</span> <span id=rcvd_trend></span></td></tr>\n")
   if(host["json"] ~= nil) then print("<tr><th><A HREF=http://en.wikipedia.org/wiki/JSON>JSON</A></th><td><i class=\"fa fa-download fa-lg\"></i> <A HREF=/lua/host_get_json.lua?host="..host_ip..">Download<A></td></tr>\n") end

   print [[
	    <tr><th>Activity Map</th><td>
	    <span id="sentHeatmap"></span>
	    <button id="sent-heatmap-prev-selector" style="margin-bottom: 10px;" class="btn"><i class="fa fa-angle-left fa-lg""></i></button>
	    <button id="heatmap-refresh" style="margin-bottom: 10px;" class="btn"><i class="fa fa-refresh fa-lg"></i></button>
	    <button id="sent-heatmap-next-selector" style="margin-bottom: 10px;" class="btn"><i class="fa fa-angle-right fa-lg"></i></button>
	    <p><span id="heatmapInfo"></span>

	    <script type="text/javascript">

	 var sent_calendar = new CalHeatMap();
        sent_calendar.init({
		       itemSelector: "#sentHeatmap",
		       data: "]]
     print("/lua/get_host_activitymap.lua?host="..host_ip..'",\n')

     timezone = get_timezone()

     now = ((os.time()-5*3600)*1000)
     today = os.time()
     today = today - (today % 86400) - 2*3600
     today = today * 1000

     print("/* "..timezone.." */\n")
     print("\t\tstart:   new Date("..now.."),\n") -- now-3h
     print("\t\tminDate: new Date("..today.."),\n")
     print("\t\tmaxDate: new Date("..(os.time()*1000).."),\n")
		     print [[
   		       domain : "hour",
		       range : 6,
		       nextSelector: "#sent-heatmap-next-selector",
		       previousSelector: "#sent-heatmap-prev-selector",

			   onClick: function(date, nb) {
					  if(nb === null) { ("#heatmapInfo").html(""); }
				       else {
					     $("#heatmapInfo").html(date + ": detected traffic for <b>" + nb + "</b> seconds ("+ Math.round((nb*100)/60)+" % of time).");
				       }
				    }

		    });

	    $(document).ready(function(){
			    $('#heatmap-refresh').click(function(){
							      sent_calendar.update(]]
									     print("\"/lua/get_host_activitymap.lua?host="..host_ip..'\");\n')
									     print [[
						    });
				      });

   </script>

	    </td></tr>
      ]]


   print("</table>\n")

   elseif((page == "packets")) then
      print [[

      <table class="table table-bordered table-striped">
	 ]]

      if(host["bytes.sent"] > 0) then
	 print('<tr><th class="text-center">Send Distribution</th><td colspan=5><div class="pie-chart" id="sizeSentDistro"></div></td></tr>')
      end
      if(host["bytes.rcvd"] > 0) then 
	 print('<tr><th class="text-center">Receive Distribution</th><td colspan=5><div class="pie-chart" id="sizeRecvDistro"></div></td></tr>')
      end

      print [[
      </table>

        <script type='text/javascript'>
	       window.onload=function() {
		   var refresh = 3000 /* ms */;
		   do_pie("#sizeSentDistro", '/lua/host_pkt_distro.lua', { type: "size", mode: "sent", ifname: "]] print(_ifname) print ('", host: ')
	print("\""..host_ip.."\" }, \"\", refresh); \n")
	print [[
		   do_pie("#sizeRecvDistro", '/lua/host_pkt_distro.lua', { type: "size", mode: "recv", ifname: "]] print(_ifname) print ('", host: ')

	print("\""..host_ip.."\" }, \"\", refresh); \n")
	print [[

		}

	    </script><p>
	]]

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
				   do_pie("#topApplicationProtocols", '/lua/host_l4_stats.lua', { ifname: "]] print(_ifname) print ('", host: ')
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
	    print("</th><td class=\"text-right\">" .. bytesToSize(sent) .. "</td><td class=\"text-right\">" .. bytesToSize(rcvd) .. "</td><td>")
	    breakdownBar(sent, "Sent", rcvd, "Rcvd")
	    print("</td><td class=\"text-right\">" .. bytesToSize(t).. "</td><td class=\"text-right\">" .. round((t * 100)/total, 2).. " %</td></tr>\n")
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
				   do_pie("#topApplicationProtocols", '/lua/iface_ndpi_stats.lua', { ifname: "]] print(_ifname) print [[" , host: ]]
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

      print("<tr><th>Total</th><td class=\"text-right\">" .. bytesToSize(host["bytes.sent"]) .. "</td><td class=\"text-right\">" .. bytesToSize(host["bytes.rcvd"]) .. "</td>")

      print("<td>")
      breakdownBar(host["bytes.sent"], "Sent", host["bytes.rcvd"], "Rcvd")
      print("</td>\n")

      print("<td colspan=2 class=\"text-right\">" ..  bytesToSize(total).. "</td></tr>\n")

      for _k in pairsByKeys(vals , desc) do
	 k = vals[_k]
	 print("<tr><th>")
	 if(host["localhost"] == true) then
	    print("<A HREF=\"/lua/host_details.lua?host=" .. host_ip .. "&page=historical&rrd_file=".. k ..".rrd\">"..k.."</A>")
	 else
	    print(k)
	 end
	 t = host["ndpi"][k]["bytes.sent"]+host["ndpi"][k]["bytes.rcvd"]
	 print("</th><td class=\"text-right\">" .. bytesToSize(host["ndpi"][k]["bytes.sent"]) .. "</td><td class=\"text-right\">" .. bytesToSize(host["ndpi"][k]["bytes.rcvd"]) .. "</td>")

	 print("<td>")
	 breakdownBar(host["ndpi"][k]["bytes.sent"], "Sent", host["ndpi"][k]["bytes.rcvd"], "Rcvd")
	 print("</td>\n")

	 print("<td class=\"text-right\">" .. bytesToSize(t).. "</td><td class=\"text-right\">" .. round((t * 100)/total, 2).. " %</td></tr>\n")
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
			     title: "Throughput",
				 field: "column_thpt",
				 sortable: true,
	 	             css: {
			        textAlign: 'right'
			     }
				 },
			     {
			     title: "Total Bytes",
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
dofile(dirs.installdir .. "/scripts/lua/inc/sankey.lua")
print("</center>")
elseif(page == "geomap") then
print("<center>")


print [[

     <style type="text/css">
     #map-canvas { width: 640px; height: 480px; }
   </style>

</center>
    <script src="https://maps.googleapis.com/maps/api/js?v=3.exp&sensor=false"></script>
<div class="container-fluid">
  <div class="row-fluid">
    <div class="span8">
      <div id="map-canvas"></div>
]]

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/geolocation_disclaimer.inc")

print [[
</div>
</div>
</div>

<script type="text/javascript">
/* IP Address to zoom */
  var zoomIP = "]] print(host_ip) print [[ ";
</script>

    <script type="text/javascript" src="/js/googleMapJson.js" ></script>
]]


elseif(page == "contacts") then

if(num > 0) then
   mode = "embed"
   if(host["name"] == nil) then host["name"] = ntop.getResolvedAddress(host["ip"]) end
   name = host["name"]
   dofile(dirs.installdir .. "/scripts/lua/hosts_interaction.lua")

   print("<table class=\"table table-bordered table-striped\">\n")
   print("<tr><th width=50%>Client Contacts (Initiator)</th><th width=50%>Server Contacts (Receiver)</th></tr>\n")

   print("<tr>")

   if(cnum  == 0) then
      print("<td>No client contacts so far</td>")
   else
      print("<td><table class=\"table table-bordered table-striped\">\n")
      print("<tr><th width=75%>Server Address</th><th>Contacts</th></tr>\n")

      -- Client
      sortTable = {}
      for k,v in pairs(host["contacts"]["client"]) do sortTable[v]=k end

      for _v,k in pairsByKeys(sortTable, rev) do
	 name = interface.getHostInfo(k)
	 v = host["contacts"]["client"][k]
	 if(name ~= nil) then
	    if(name["name"] ~= nil) then n = name["name"] else n = ntop.getResolvedAddress(name["ip"]) end
	    url = "<A HREF=\"/lua/host_details.lua?host="..k.."\">"..n.."</A>"
	 else
	    url = k
	 end
	 print("<tr><th>"..url.."</th><td class=\"text-right\">" .. formatValue(v) .. "</td></tr>\n")
      end
      print("</table></td>\n")
   end

   if(snum  == 0) then
      print("<td>No server contacts so far</td>")
   else
      print("<td><table class=\"table table-bordered table-striped\">\n")
      print("<tr><th width=75%>Client Address</th><th>Contacts</th></tr>\n")

      -- Server
      sortTable = {}
      for k,v in pairs(host["contacts"]["server"]) do sortTable[v]=k end

      for _v,k in pairsByKeys(sortTable, rev) do
	 name = interface.getHostInfo(k)
	 v = host["contacts"]["server"][k]
	 if(name ~= nil) then
	    if(name["name"] ~= nil) then n = name["name"] else n = ntop.getResolvedAddress(name["ip"]) end
	    url = "<A HREF=\"/lua/host_details.lua?host="..k.."\">"..n.."</A>"
	 else
	    url = k
	 end
	 print("<tr><th>"..url.."</th><td class=\"text-right\">" .. formatValue(v) .. "</td></tr>\n")
      end
      print("</table></td>\n")
   end

print("</tr>\n")

print("</table>\n")
else
   print("No contacts for this host")
end


elseif(page == "historical") then
if(_GET["rrd_file"] == nil) then
   rrdfile = "bytes.rrd"
else
   rrdfile=_GET["rrd_file"]
end

drawRRD(ifname, host_ip, rrdfile, _GET["graph_zoom"], '/lua/host_details.lua?host='..host_ip..'&page=historical', 1, _GET["epoch"])


elseif(page == "aggregations") then
print [[
      <div id="table-hosts"></div>
	 <script>
	 $("#table-hosts").datatable({
					url: "/lua/get_hosts_data.lua?aggregated=1]]
					print("&client="..host_ip)
print [[",
	       showPagination: true,
	       buttons: [ '<div class="btn-group"><button class="btn dropdown-toggle" data-toggle="dropdown">Aggregations<span class="caret"></span></button> <ul class="dropdown-menu">]]

print('<li><a href="/lua/aggregated_hosts_stats.lua">All</a></li>')

families = interface.getAggregationFamilies()
for key,v in pairs(families) do
   print('<li><a href="/lua/host_details.lua?host='.. host_ip ..'&page=aggregations&protocol=' .. v..'">'..key..'</a></li>')
end 

print("</ul> </div>' ],")
print("title: \"Client Host Aggregations\",\n")

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/aggregated_hosts_stats_top.inc")

prefs = ntop.getPrefs()

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/aggregated_hosts_stats_bottom.inc")



else
   print(page)
end
end

print [[
<script>
/*
      $(document).ready(function() {
	      $('.progress .bar').progressbar({ use_percentage: true, display_text: 1 });
   });
*/


//var thptChart = $("#thpt_load_chart").peity("line", { width: 64 });

]]

print("var last_pkts_sent = " .. host["pkts.sent"] .. ";\n")
print("var last_pkts_rcvd = " .. host["pkts.rcvd"] .. ";\n")

print [[
setInterval(function() {
	  $.ajax({
		    type: 'GET',
		    url: '/lua/host_stats.lua',
		    data: { ifname: "]] print(_ifname) print [[", host: "]] print(host_ip) print [[" },
		    /* error: function(content) { alert("JSON Error: inactive host purged or ntopng terminated?"); }, */
		    success: function(content) {
			var host = jQuery.parseJSON(content);
			$('#first_seen').html(epoch2Seen(host["seen.first"]));
			$('#last_seen').html(epoch2Seen(host["seen.last"]));
			$('#pkts_sent').html(formatPackets(host["pkts.sent"]));
			$('#pkts_rcvd').html(formatPackets(host["pkts.rcvd"]));
			$('#bytes_sent').html(bytesToVolume(host["bytes.sent"]));
			$('#bytes_rcvd').html(bytesToVolume(host["bytes.rcvd"]));
			$('#name').html(host["name"]);

			/* **************************************** */

			if(last_pkts_sent == host["pkts.sent"]) {
			   $('#sent_trend').html("<i class=\"fa fa-minus\"></i>");
			} else {
			   $('#sent_trend').html("<i class=\"fa fa-arrow-up\"></i>");
			}

			if(last_pkts_rcvd == host["pkts.rcvd"]) {
			   $('#rcvd_trend').html("<i class=\"fa fa-minus\"></i>");
			} else {
			   $('#rcvd_trend').html("<i class=\"fa fa-arrow-up\"></i>");
			}

			last_pkts_sent = host["pkts.sent"];
			last_pkts_rcvd = host["pkts.rcvd"];

			/* **************************************** */

			/*
			$('#throughput').html(rsp.throughput);

			var values = thptChart.text().split(",");
			values.shift();
			values.push(rsp.throughput_raw);
			thptChart.text(values.join(",")).change();
			*/
		     }
	           });
		 }, 3000);

</script>
 ]]


dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
