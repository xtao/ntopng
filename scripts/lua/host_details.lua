--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "graph_utils"
require "alert_utils"

page        = _GET["page"]
host_ip     = _GET["host"]
protocol_id = _GET["protocol"]


active_page = "hosts"

if(host_ip == nil) then
   sendHTTPHeader('text/html')
   ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")
   dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")
   print("<div class=\"alert alert-error\"><img src=/img/warning.png> Host parameter is missing (internal error ?)</div>")
   return
end

if(protocol_id == nil) then protocol_id = "" end

_ifname = tostring(interface.name2id(ifname))
interface.find(ifname)

--ip_elems = split(host_ip, " ");
--host_ip = ip_elems[1]
host = nil
family = nil

--print(">>>") print(host_ip) print("<<<")

--if(ip_elems[2] == nil) then
   host = interface.getHostInfo(host_ip)
   restoreFailed = false

   if((host == nil) and (_GET["mode"] == "restore")) then
      interface.restoreHost(host_ip)
      host = interface.getHostInfo(host_ip)
      restoreFailed = true
   end
--else
--   family = ip_elems[2]
--end

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

   if(_GET["custom_name"] ~=nil) then
      ntop.setHashCache("ntop.alternate_names", host_ip, _GET["custom_name"])
   end

   host["alternate_name"] = ntop.getHashCache("ntop.alternate_names", host_ip)

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
  print("<li class=\"active\"><a href=\"#\"><i class=\"fa fa-home fa-lg\"></i></a></li>\n")
else
  print("<li><a href=\""..url.."&page=overview\"><i class=\"fa fa-home fa-lg\"></i></a></li>")
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

if(page == "dns") then
  print("<li class=\"active\"><a href=\"#\">DNS</a></li>\n")
else
   if((host["dns"] ~= nil)
   and ((host["dns"]["sent"]["num_queries"]+host["dns"]["rcvd"]["num_queries"]) > 0)) then
      print("<li><a href=\""..url.."&page=dns\">DNS</a></li>")
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

if(page == "pearson") then
  print("<li class=\"active\"><a href=\"#\">Correlation</a></li>\n")
else
   if((host["ip"] ~= nil) and (host["privatehost"] == false)) then
      print("<li><a href=\""..url.."&page=pearson\">Correlation</a></li>")
   end
end

if(page == "jaccard") then
  print("<li class=\"active\"><a href=\"#\">Similarity</a></li>\n")
else
   if((host["ip"] ~= nil) and (host["privatehost"] == false)) then
      print("<li><a href=\""..url.."&page=jaccard\">Similarity</a></li>")
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
      print("\n<li class=\"active\"><a href=\"#\">Current Contacts</a></li>\n")
   else
      print("\n<li><a href=\""..url.."&page=contacts\">Current Contacts</a></li>")
   end
end

t = os.time()
when = os.date("%y%m%d", t)
base_name = when.."|"..ifname.."|"..ntop.getHostId(host_ip)
keyname = base_name.."|contacted_peers"
v1 = ntop.getHashKeysCache(keyname)
--print(keyname.."\n")

if(v1 == nil) then
   keyname = base_name.."|contacted_by"
   v1 = ntop.getHashKeysCache(keyname)
end

if(v1 ~= nil) then
   if(page == "todays_contacts") then
      print("\n<li class=\"active\"><a href=\"#\">Today's Contacts</a></li>\n")
   else
      print("\n<li><a href=\""..url.."&page=todays_contacts\">Today's Contacts</a></li>")
   end
end

if(getItemsNumber(interface.getAggregatedHostsInfo(0, host_ip)) > 0) then
   if(page == "aggregations") then
      print("\n<li class=\"active\"><a href=\"#\">Aggregations</a></li>\n")
   else
      print("\n<li><a href=\""..url.."&page=aggregations\">Aggregations</a></li>")
   end
end

if(host["ip"] ~= nil) then
   if(page == "alerts") then
      print("\n<li class=\"active\"><a href=\"#\"><i class=\"fa fa-warning fa-lg\"></i></a></li>\n")
   else
      print("\n<li><a href=\""..url.."&page=alerts\"><i class=\"fa fa-warning fa-lg\"></i></a></li>")
   end
end

if(ntop.exists(rrdname)) then
if(page == "historical") then
  print("\n<li class=\"active\"><a href=\"#\">Historical</a></li>\n")
else
  print("\n<li><a href=\""..url.."&page=historical\">Historical</a></li>")
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
	 print("<tr><th width=35%>(Router) MAC Address</th><td colspan=2>" .. host["mac"].. "</td></tr>\n")
      end
      print("<tr><th>IP Address</th><td colspan=1>" .. host["ip"])
   else
      if(host["mac"] ~= nil) then
	 print("<tr><th>MAC Address</th><td colspan=1>" .. host["mac"].. "</td></tr>\n")
      end
   end

   if((host["city"] ~= "") or (host["country"] ~= "")) then
      print(" [ " .. host["city"] .. " <img src=\"/img/blank.gif\" class=\"flag flag-".. string.lower(host["country"]) .."\"> ]")
   end

trigger_alerts = _GET["trigger_alerts"]

if(trigger_alerts ~= nil) then
   if(trigger_alerts == "true") then
      ntop.delHashCache("ntopng.prefs.alerts", host_ip)
   else
      ntop.setHashCache("ntopng.prefs.alerts", host_ip, trigger_alerts)
   end
end

suppressAlerts = ntop.getHashCache("ntopng.prefs.alerts", host_ip)
if((suppressAlerts == "") or (suppressAlerts == nil) or (suppressAlerts == "true")) then
   checked = 'checked="checked"'
   value = "false" -- Opposite
else
   checked = ""
   value = "true" -- Opposite
end



if(host["ip"] ~= nil) then

print [[
</td>
<td>
<form id="alert_prefs" class="form-inline" style="margin-bottom: 0px;">
	 <input type="hidden" name="host" value="]]

      print(host_ip)
      print('"><input type="hidden" name="trigger_alerts" value="'..value..'"><input type="checkbox" value="1" '..checked..' onclick="this.form.submit();"> Trigger Host Alerts</input>')
      print('</form></td></tr>')
   end

   if((host["vlan"] ~= nil) and (host["vlan"] > 0)) then print("<tr><th>VLAN Id</th><td colspan=2>"..host["vlan"].."</td></tr>\n") end
   if(host["os"] ~= "") then print("<tr><th>OS</th><td colspan=2>" .. mapOS2Icon(host["os"]) .. " </td></tr>\n") end
   if((host["asn"] ~= nil) and (host["asn"] > 0)) then print("<tr><th>ASN</th><td colspan=2>".. printASN(host["asn"], host.asname) .. " [ " .. host.asname .. " ] </td></tr>\n") end

   if((host["category"] ~= nil) and (host["category"] ~= "")) then
      cat = getCategory(host["category"])

      if(cat ~= "") then
	 print("<tr><th>Category</th><td colspan=2>".. cat .."</td></tr>\n")
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
      print("</td>\n")
   end

if(host["ip"] ~= nil) then
print [[
<td>
<form class="form-inline" style="margin-bottom: 0px;">
	 <input type="hidden" name="host" value="]]
      print(host_ip)
print [[">
	 <input type="text" name="custom_name" placeholder="Custom Name" value="]]
      if(host["alternate_name"] ~= nil) then print(host["alternate_name"]) end
print [["></input>
  <button type="submit" class="btn">Save Name</button>
</form>
</td></tr>
   ]]
end


if(host["num_alerts"] > 0) then
   print("<tr><th><i class=\"fa fa-warning fa-lg\" style='color: #B94A48;'></i>  <A HREF=/lua/show_alerts.lua>Alerts</A></th><td colspan=2></li> <span id=num_alerts>"..host["num_alerts"] .. "</span> <span id=alerts_trend></span></td></tr>\n")
end

   print("<tr><th>First / Last Seen</th><td><span id=first_seen>" .. formatEpoch(host["seen.first"]) ..  " [" .. secondsToTime(os.time()-host["seen.first"]) .. " ago]" .. "</span></td>\n")
   print("<td><span id=last_seen>" .. formatEpoch(host["seen.last"]) .. " [" .. secondsToTime(os.time()-host["seen.last"]) .. " ago]" .. "</span></td></tr>\n")


   if((host["bytes.sent"]+host["bytes.rcvd"]) > 0) then
      print("<tr><th>Sent vs Received Traffic Breakdown</th><td colspan=2>")
      breakdownBar(host["bytes.sent"], "Sent", host["bytes.rcvd"], "Rcvd")
      print("</td></tr>\n")
   end

   print("<tr><th>Traffic Sent / Received</th><td><span id=pkts_sent>" .. formatPackets(host["pkts.sent"]) .. "</span> / <span id=bytes_sent>".. bytesToSize(host["bytes.sent"]) .. "</span> <span id=sent_trend></span></td><td><span id=pkts_rcvd>" .. formatPackets(host["pkts.rcvd"]) .. "</span> / <span id=bytes_rcvd>".. bytesToSize(host["bytes.rcvd"]) .. "</span> <span id=rcvd_trend></span></td></tr>\n")

   print("<tr><th>Flows 'As Client' / 'As Server'</th><td><span id=flows_as_client>" .. formatValue(host["flows.as_client"]) .. "</span> <span id=as_client_trend></span></td><td><span id=flows_as_server>" .. formatValue(host["flows.as_server"]) .. "</span> <span id=as_server_trend></td></tr>\n")

   if(host["json"] ~= nil) then print("<tr><th><A HREF=http://en.wikipedia.org/wiki/JSON>JSON</A></th><td colspan=2><i class=\"fa fa-download fa-lg\"></i> <A HREF=/lua/host_get_json.lua?host="..host_ip..">Download<A></td></tr>\n") end

   print [[
	    <tr><th>Activity Map</th><td colspan=2>
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

   elseif(page == "dns") then
      if(host["dns"] ~= nil) then
	 print("<table class=\"table table-bordered table-striped\">\n")
	 print("<tr><th>DNS Breakdown</th><th>Queries</th><th>Positive Replies</th><th>Error Replies</th><th colspan=2>Reply Breakdown</th></tr>")
	 print("<tr><th>Sent</th><td class=\"text-right\"><span id=dns_sent_num_queries>".. formatValue(host["dns"]["sent"]["num_queries"]) .."</span> <span id=trend_sent_num_queries></span></td>")
	 print("<td class=\"text-right\"><span id=dns_sent_num_replies_ok>".. formatValue(host["dns"]["sent"]["num_replies_ok"]) .."</span> <span id=trend_sent_num_replies_ok></span></td>")
	 print("<td class=\"text-right\"><span id=dns_sent_num_replies_error>".. formatValue(host["dns"]["sent"]["num_replies_error"]) .."</span> <span id=trend_sent_num_replies_error></span></td><td colspan=2>")
	 breakdownBar(host["dns"]["sent"]["num_replies_ok"], "OK", host["dns"]["sent"]["num_replies_error"], "Error")
	 print("</td></tr>")

	 if(host["dns"]["sent"]["num_queries"] > 0) then
	    print [[         
		     <tr><th>DNS Query Sent Distribution</th><td colspan=5>
		     <div class="pie-chart" id="dnsSent"></div>
		     <script type='text/javascript'>
					 var refresh = 3000 /* ms */;
					 do_pie("#dnsSent", '/lua/host_dns_breakdown.lua', { host: "]] print(host_ip) print [[", mode: "sent" }, "", refresh);
				      </script>
					 </td></tr>
           ]]
         end


	 print("<tr><th>Rcvd</th><td class=\"text-right\"><span id=dns_rcvd_num_queries>".. formatValue(host["dns"]["rcvd"]["num_queries"]) .."</span> <span id=trend_rcvd_num_queries></span></td>")
	 print("<td class=\"text-right\"><span id=dns_rcvd_num_replies_ok>".. formatValue(host["dns"]["rcvd"]["num_replies_ok"]) .."</span> <span id=trend_rcvd_num_replies_ok></span></td>")
	 print("<td class=\"text-right\"><span id=dns_rcvd_num_replies_error>".. formatValue(host["dns"]["rcvd"]["num_replies_error"]) .."</span> <span id=trend_rcvd_num_replies_error></span></td><td colspan=2>")
	 breakdownBar(host["dns"]["rcvd"]["num_replies_ok"], "OK", host["dns"]["rcvd"]["num_replies_error"], "Error")
	 print("</td></tr>")

	 if(host["dns"]["rcvd"]["num_queries"] > 0) then
print [[         
	 <tr><th>DNS Rcvd Query Distribution</th><td colspan=5>
         <div class="pie-chart" id="dnsRcvd"></div>
         <script type='text/javascript'>
         var refresh = 3000 /* ms */;
	     do_pie("#dnsRcvd", '/lua/host_dns_breakdown.lua', { host: "]] print(host_ip) print [[", mode: "rcvd" }, "", refresh);
         </script>
         </td></tr>
]]
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
   elseif(page == "todays_contacts") then

   t = os.time()
   when = os.date("%y%m%d", t)
   base_name = when.."|"..ifname.."|"..ntop.getHostId(host_ip)
   keyname = base_name.."|contacted_peers"
   --io.write(keyname.."\n")
   -- print(keyname.."\n")
   protocols = {}
   protocols[65535] = interface.getNdpiProtoName(65535)
   v1 = ntop.getHashKeysCache(keyname)
   if(v1 ~= nil) then
      for k,_ in pairs(v1) do
	 v = ntop.getHashCache(keyname, k)
	 --io.write(v.."\n")
	 if(v ~= nil) then
	    values = split(k, "@");
	    protocol = tonumber(values[2])

	    -- 254 is OperatingSystem
	    if((protocols[protocol] == nil) and (protocol ~= 254)) then
	       protocols[protocol] = interface.getNdpiProtoName(protocol)
	    end
	 end
      end
   end

print [[
      <div id="table-hosts"></div>
	 <script>
	 $("#table-hosts").datatable({
				  ]]
				  print("url: \"/lua/get_host_contacts.lua?host=" .. host_ip.."&protocol="..protocol_id.."\",\n")
print [[
	       buttons: [ '<div class="btn-group"><button class="btn dropdown-toggle" data-toggle="dropdown">Type/Protocol<span class="caret"></span></button> <ul class="dropdown-menu">]]
url = "/lua/host_details.lua?host="..host_ip.."&page=todays_contacts"
print('<li><a href="'.. url ..'">All</a></li>')

for key,v in pairs(protocols) do
   print('<li><a href="'..url..'&protocol=' .. key..'">'..v..'</a></li>')
end

print("</ul> </div>' ],\n")

print [[
	       showPagination: true,
	       title: "Today's Contacts",
	        columns: [
			     {
			     title: "Host",
				 field: "column_ip",
				sortable: true,
	 	             css: {
			        textAlign: 'left'
			     }
				 },
			     {
			     title: "Name",
				 field: "column_name",
				 sortable: false,
	 	             css: {
			        textAlign: 'left'
			     }
				 },
			     {
			     title: "Contact Type/Protocol",
				 field: "column_protocol",
				 sortable: true,
	 	             css: {
			        textAlign: 'center'
			     }
				 },
			     {
			     title: "Contacts Number",
				 field: "column_num_contacts",
				 sortable: true,
	 	             css: {
			        textAlign: 'right'
			     }
				 }
			     ]
	       });
       </script>

   ]]

print("<i class=\"fa fa-download fa-lg\"></i> <A HREF='/lua/get_host_contacts.lua?format=json&host=" .. host_ip.."'>Download "..host_ip.." contacts as JSON<A>\n")
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
    <script src="/js/markerclusterer.js"></script>
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

elseif(page == "pearson") then

print [[
<div id="prg" class="container">
    <div class="progress progress-striped active">
	 <div class="bar" style="width: 100%;"></div>
    </div>
</div>
]]

pearson = interface.correlateHostActivity(host_ip)

print [[
<script type="text/javascript">
  var $bar = $('#prg');

  $bar.hide();
  $bar.remove();
</script>
]]

vals = {}
for k,v in pairs(pearson) do
   vals[v] = k
end

max_hosts = 10

n = 0

if(host["name"] == nil) then host["name"] = ntop.getResolvedAddress(host["ip"]) end

for v,k in pairsByKeys(vals, rev) do
   if(v > 0) then
      if(n == 0) then
	 print("<table class=\"table table-bordered\">\n")
	 print("<tr><th>Local Hosts similar to ".. host["name"] .."</th><th>Correlation Coefficient</th></tr>\n")
      end

      correlated_host = interface.getHostInfo(k)
      
      if(correlated_host["name"] == nil) then correlated_host["name"] = ntop.getResolvedAddress(correlated_host["ip"]) end
      print("<tr><th align=left><A HREF=/lua/host_details.lua?host="..k..">"..correlated_host["name"].."</a></th><td class=\"text-right\">"..round(tofloat(v), 3).."</td></tr>\n")
      n = n +1
      
      if(n >= max_hosts) then
	 break
      end
   end
end

if(n > 0) then
   print("</table>\n")
else
   print("There is no host correlated to ".. host["name"].."<p>\n")
end

print [[
<b>Note</b>:
<ul>
	 <li>Correleation considers only activity map as shown in the <A HREF=/lua/host_details.lua?host=]] print(host_ip) print [[>host overview</A>.
<li>Two hosts are correlated when their network behaviour is close. In particular when their activity map is very similar. The <A HREF=http://en.wikipedia.org/wiki/Pearson_product-moment_correlation_coefficient>correlation coefficient</A> is a number between +1 and -1, where +1 means that two hosts are correlated, 0 means that they have no particular correlation, and -1 that they behave in an opposite way.
</ul>
]]

elseif(page == "jaccard") then

print [[
<div id="prg" class="container">
    <div class="progress progress-striped active">
	 <div class="bar" style="width: 100%;"></div>
    </div>
</div>
]]

jaccard = interface.similarHostActivity(host_ip)

print [[
<script type="text/javascript">
  var $bar = $('#prg');

  $bar.hide();
  $bar.remove();
</script>
]]

vals = {}
for k,v in pairs(jaccard) do
   vals[v] = k
end

max_hosts = 10

n = 0

if(host["name"] == nil) then host["name"] = ntop.getResolvedAddress(host["ip"]) end

for v,k in pairsByKeys(vals, rev) do
   if(v > 0) then
      if(n == 0) then
	 print("<table class=\"table table-bordered\">\n")
	 print("<tr><th>Local Hosts Similar to ".. host["name"] .."</th><th>Jaccard Coefficient</th></tr>\n")
      end

      correlated_host = interface.getHostInfo(k)
      
      if(correlated_host["name"] == nil) then correlated_host["name"] = ntop.getResolvedAddress(correlated_host["ip"]) end
      print("<tr><th align=left><A HREF=/lua/host_details.lua?host="..k..">"..correlated_host["name"].."</a></th><td class=\"text-right\">"..v.."</td></tr>\n")
      n = n +1
      
      if(n >= max_hosts) then
	 break
      end
   end
end

if(n > 0) then
   print("</table>\n")
else
   print("There is no host correlated to ".. host["name"].."<p>\n")
end

print [[
<b>Note</b>:
<ul>
	 <li>Jaccard Similarity considers only activity map as shown in the <A HREF=/lua/host_details.lua?host=]] print(host_ip) print [[>host overview</A>.
<li>Two hosts are similar according to the Jaccard coefficient when their activity tends to overlap. In particular when their activity map is very similar. The <A HREF=http://en.wikipedia.org/wiki/Jaccard_index>Jaccard similarity coefficient</A> is a number between +1 and 0.
</ul>
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

      num = 0
      max_num = 64 -- Do not create huge maps
      for _v,k in pairsByKeys(sortTable, rev) do
	 if(num >= max_num) then break end
	 num = num + 1
	 name = interface.getHostInfo(k)
	 v = host["contacts"]["client"][k]
	 if(name ~= nil) then
	    if(name["name"] ~= nil) then n = name["name"] else n = ntop.getResolvedAddress(name["ip"]) end
	    url = "<A HREF=\"/lua/host_details.lua?host="..k.."\">"..n.."</A>"
	 else
	    url = k
	 end

	 info = interface.getHostInfo(k)
	 if(info ~= nil) then
	    if((info["country"] ~= nil) and (info["country"] ~= "")) then
	       url = url .." <img src='/img/blank.gif' class='flag flag-".. string.lower(info["country"]) .."'> "
	    end
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
	 info = interface.getHostInfo(k)
	 if(info ~= nil) then
	    if((info["country"] ~= nil) and (info["country"] ~= "")) then
	       url = url .." <img src='/img/blank.gif' class='flag flag-".. string.lower(info["country"]) .."'> "
	    end
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

elseif(page == "alerts") then

local tab = _GET["tab"]

if(tab == nil) then tab = alerts_granularity[1][1] end

print [[ <ul class="nav nav-tabs">
]]

for _,e in pairs(alerts_granularity) do
   k = e[1]
   l = e[2]

   if(k == tab) then print("\t<li class=active>") else print("\t<li>") end
   print("<a href=\"/lua/host_details.lua?host="..host_ip.."&page=alerts&tab="..k.."\">"..l.."</a></li>\n")
end

-- Before doing anything we need to check if we need to save values

vals = { }
alerts = ""
to_save = false

if((_GET["to_delete"] ~= nil) and (_GET["SaveAlerts"] == nil)) then
   delete_host_alert_configuration(host_ip)
   alerts = nil
else
   for k,_ in pairs(alert_functions_description) do
      value    = _GET["value-"..k]
      operator = _GET["operator-"..k]

      if((value ~= nil) and (operator ~= nil)) then
	 to_save = true
	 value = tonumber(value)
	 if(value ~= nil) then 
	    if(alerts ~= "") then alerts = alerts .. "," end
	    alerts = alerts .. k .. ";" .. operator .. ";" .. value
	 end
      end
   end

   --print(alerts)

   if(to_save) then
      if(alerts == "") then
	 ntop.delHashCache("ntopng.prefs.alerts_"..tab, host["ip"])
      else
	 ntop.setHashCache("ntopng.prefs.alerts_"..tab, host["ip"], alerts)
      end
   else
      alerts = ntop.getHashCache("ntopng.prefs.alerts_"..tab, host["ip"])
   end
end

if(alerts ~= nil) then
   --print(alerts)   
   --tokens = string.split(alerts, ",")
   tokens = split(alerts, ",")

   --print(tokens)
   if(tokens ~= nil) then
      for _,s in pairs(tokens) do
	 t = string.split(s, ";")
	 --print("-"..t[1].."-")
	 vals[t[1]] = { t[2], t[3] }
      end
   end
end

print [[
 </ul>
 <table id="user" class="table table-bordered table-striped" style="clear: both"> <tbody>
 <tr><th width=20%>Alert Function</th><th>Threshold</th></tr>


<form>
 <input type=hidden name=page value=alerts>
]]

print("<input type=hidden name=host value=\""..host_ip.."\">\n")
print("<input type=hidden name=tab value="..tab..">\n")

for k,v in pairsByKeys(alert_functions_description, asc) do
   print("<tr><th>"..k.."</th><td>\n")
   print("<select name=operator-".. k ..">\n")
   if((vals[k] ~= nil) and (vals[k][1] == "gt")) then print("<option selected=\"selected\"") else print("<option ") end
   print("value=\"gt\">&gt;</option>\n")

   if((vals[k] ~= nil) and (vals[k][1] == "eq")) then print("<option selected=\"selected\"") else print("<option ") end
   print("value=\"eq\">=</option>\n")

   if((vals[k] ~= nil) and (vals[k][1] == "lt")) then print("<option selected=\"selected\"") else print("<option ") end
   print("value=\"lt\">&lt;</option>\n")
   print("</select>\n")
   print("<input type=text name=\"value-"..k.."\" value=\"")
   if(vals[k] ~= nil) then print(vals[k][2]) end
   print("\">\n\n")
   print("<br><small>"..v.."</small>\n")
   print("</td></tr>\n")
end

print [[
<tr><th colspan=2  style="text-align: center; white-space: nowrap;" >

<input type="submit" class="btn btn-primary" name="SaveAlerts" value="Save Configuration">

<a href="#myModal" role="button" class="btn" data-toggle="modal"><i type="submit" class="fa fa-trash-o"></i> Delete All Configured Alerts</button></a>
<!-- Modal -->
<div id="myModal" class="modal hide fade" tabindex="-1" role="dialog" aria-labelledby="myModalLabel" aria-hidden="true">
  <div class="modal-header">
    <button type="button" class="close" data-dismiss="modal" aria-hidden="true">X</button>
    <h3 id="myModalLabel">Confirm Action</h3>
  </div>
  <div class="modal-body">
	 <p>Do you really want to delele all configured alerts for host ]] print(host_ip) print [[?</p>
  </div>
  <div class="modal-footer">
    <form class=form-inline style="margin-bottom: 0px;" method=get action="#"><input type=hidden name=to_delete value="__all__">
    <button class="btn" data-dismiss="modal" aria-hidden="true">Close</button>
    <button class="btn btn-primary" type="submit">Delete All</button>

  </div>
</form>


</th> </tr>



</tbody> </table>
]]






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
					print("&protocol="..protocol_id.."&client="..host_ip)
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
print("var last_num_alerts = " .. host["num_alerts"] .. ";\n")
print("var last_flows_as_server = " .. host["flows.as_server"] .. ";\n")
print("var last_flows_as_client = " .. host["flows.as_client"] .. ";\n")

if(host["dns"] ~= nil) then
   print("var last_dns_sent_num_queries = " .. host["dns"]["sent"]["num_queries"] .. ";\n")
   print("var last_dns_sent_num_replies_ok = " .. host["dns"]["sent"]["num_replies_ok"] .. ";\n")
   print("var last_dns_sent_num_replies_error = " .. host["dns"]["sent"]["num_replies_error"] .. ";\n")
   print("var last_dns_rcvd_num_queries = " .. host["dns"]["rcvd"]["num_queries"] .. ";\n")
   print("var last_dns_rcvd_num_replies_ok = " .. host["dns"]["rcvd"]["num_replies_ok"] .. ";\n")
   print("var last_dns_rcvd_num_replies_error = " .. host["dns"]["rcvd"]["num_replies_error"] .. ";\n")
end

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
			$('#num_alerts').html(host["num_alerts"]);

			$('#flows_as_client').html(addCommas(host["flows.as_client"]));
			$('#flows_as_server').html(addCommas(host["flows.as_server"]));
		  ]]

if(host["dns"] ~= nil) then
print [[
			   $('#dns_sent_num_queries').html(addCommas(host["dns"]["sent"]["num_queries"]));
			   $('#dns_sent_num_replies_ok').html(addCommas(host["dns"]["sent"]["num_replies_ok"]));
			   $('#dns_sent_num_replies_error').html(addCommas(host["dns"]["sent"]["num_replies_error"]));
			   $('#dns_rcvd_num_queries').html(addCommas(host["dns"]["rcvd"]["num_queries"]));
			   $('#dns_rcvd_num_replies_ok').html(addCommas(host["dns"]["rcvd"]["num_replies_ok"]));
			   $('#dns_rcvd_num_replies_error').html(addCommas(host["dns"]["rcvd"]["num_replies_error"]));

			   if(host["dns"]["sent"]["num_queries"] == last_dns_sent_num_queries) {
			      $('#trend_sent_num_queries').html("<i class=\"fa fa-minus\"></i>");
			   } else {
			      last_dns_sent_num_queries = host["dns"]["sent"]["num_queries"];
			      $('#trend_sent_num_queries').html("<i class=\"fa fa-arrow-up\"></i>");
			   }

			   if(host["dns"]["sent"]["num_replies_ok"] == last_dns_sent_num_replies_ok) {
			      $('#trend_sent_num_replies_ok').html("<i class=\"fa fa-minus\"></i>");
			   } else {
			      last_dns_sent_num_replies_ok = host["dns"]["sent"]["num_replies_ok"];
			      $('#trend_sent_num_replies_ok').html("<i class=\"fa fa-arrow-up\"></i>");
			   }

			   if(host["dns"]["sent"]["num_replies_error"] == last_dns_sent_num_replies_error) {
			      $('#trend_sent_num_replies_error').html("<i class=\"fa fa-minus\"></i>");
			   } else {
			      last_dns_sent_num_replies_error = host["dns"]["sent"]["num_replies_error"];
			      $('#trend_sent_num_replies_error').html("<i class=\"fa fa-arrow-up\"></i>");
			   }

			   if(host["dns"]["rcvd"]["num_queries"] == last_dns_rcvd_num_queries) {
			      $('#trend_rcvd_num_queries').html("<i class=\"fa fa-minus\"></i>");
			   } else {
			      last_dns_rcvd_num_queries = host["dns"]["rcvd"]["num_queries"];
			      $('#trend_rcvd_num_queries').html("<i class=\"fa fa-arrow-up\"></i>");
			   }

			   if(host["dns"]["rcvd"]["num_replies_ok"] == last_dns_rcvd_num_replies_ok) {
			      $('#trend_rcvd_num_replies_ok').html("<i class=\"fa fa-minus\"></i>");
			   } else {
			      last_dns_rcvd_num_replies_ok = host["dns"]["rcvd"]["num_replies_ok"];
			      $('#trend_rcvd_num_replies_ok').html("<i class=\"fa fa-arrow-up\"></i>");
			   }

			   if(host["dns"]["rcvd"]["num_replies_error"] == last_dns_rcvd_num_replies_error) {
			      $('#trend_rcvd_num_replies_error').html("<i class=\"fa fa-minus\"></i>");
			   } else {
			      last_dns_rcvd_num_replies_error = host["dns"]["rcvd"]["num_replies_error"];
			      $('#trend_rcvd_num_replies_error').html("<i class=\"fa fa-arrow-up\"></i>");
			   }
		     ]]
end

print [[
			/* **************************************** */

			if(host["flows.as_client"] == last_flows_as_client) {
			   $('#as_client_trend').html("<i class=\"fa fa-minus\"></i>");
			} else {
			   $('#as_client_trend').html("<i class=\"fa fa-arrow-up\"></i>");
			}

			if(host["flows.as_server"] == last_flows_as_server) {
			   $('#as_server_trend').html("<i class=\"fa fa-minus\"></i>");
			} else {
			   $('#as_server_trend').html("<i class=\"fa fa-arrow-up\"></i>");
			}

			if(last_num_alerts == host["num_alerts"]) {
			   $('#alerts_trend').html("<i class=\"fa fa-minus\"></i>");
			} else {
			   $('#alerts_trend').html("<i class=\"fa fa-arrow-up\" style=\"color: #B94A48;\"></i>");
			}

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

			last_num_alerts = host["num_alerts"];
			last_pkts_sent = host["pkts.sent"];
			last_pkts_rcvd = host["pkts.rcvd"];
			last_flows_as_server = host["flows.as_server"];
			last_flows_as_client = host["flows.as_client"];
		  ]]


print [[

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
