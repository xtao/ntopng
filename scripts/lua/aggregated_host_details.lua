--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "graph_utils"

page = _GET["page"]
if(page == nil) then page = "overview" end

host_ip = _GET["host"]

sendHTTPHeader('text/html')
ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

if(host_ip == nil) then
   print("<div class=\"alert alert-error\"><img src=/img/warning.png> Host parameter is missing (internal error ?)</div>")
   return
end

interface.find(ifname)
host = interface.getAggregatedHostInfo(host_ip)

if(host == nil) then
   print("<div class=\"alert alert-error\"><img src=/img/warning.png> Unable to find "..host_ip.." (data expired ?)</div>")
   return
else
print [[
<div class="bs-docs-example">
            <div class="navbar">
              <div class="navbar-inner">
<ul class="nav">
]]

url="/lua/aggregated_host_details.lua?host="..host_ip

print("<li><a href=\"#\">"..host_ip.." </a></li>\n")

if(page == "overview") then
  print("<li class=\"active\"><a href=\"#\">Overview</a></li>\n")
else
  print("<li><a href=\""..url.."&page=overview\">Overview</a></li>")
end

num = 0
if(host.contacts ~= nil) then
   for k,v in pairs(host["contacts"]["client"]) do num = num + 1 end
   for k,v in pairs(host["contacts"]["server"]) do num = num + 1 end
end

if(num > 0) then 
   if(page == "contacts") then
      print("<li class=\"active\"><a href=\"#\">Host Contacts</a></li>\n")
   else
      print("<li><a href=\""..url.."&page=contacts\">Host Contacts</a></li>")
   end
end

print [[
</ul>
</div>
</div>
</div>
   ]]

--print("<b>".._GET["page"].."</b>")
if(page == "overview") then
   print("<table class=\"table table-bordered\">\n")
   print("<tr><th>Name</th><td><A HREF=http://" .. host["name"].. ">".. host["name"].."</A></td></tr>\n")
   print("<tr><th>First Seen</th><td>" .. formatEpoch(host["seen.first"]) ..  " [" .. secondsToTime(os.time()-host["seen.first"]) .. " ago]" .. "</td></tr>\n")
   print("<tr><th>Last Seen</th><td><div id=last_seen>" .. formatEpoch(host["seen.last"]) .. " [" .. secondsToTime(os.time()-host["seen.last"]) .. " ago]" .. "</div></td></tr>\n")

   print("<tr><th>Contacts Received</th><td><span id=contacts>" .. formatValue(host["pkts.rcvd"]) .. "</span> <span id=contacts_trend></span></td></tr>\n")
   print("</table>\n")

elseif(page == "contacts") then


if(num > 0) then
print("<table class=\"table table-bordered table-striped\">\n")
print("<tr><th>Contacted by</th><th>Contacts Number</th></tr>\n")

-- Client
sortTable = {}
for k,v in pairs(host["contacts"]["client"]) do sortTable[v]=k end

for _v,k in pairsByKeys(sortTable, rev) do 
   name = interface.getHostInfo(k)
   v = host["contacts"]["client"][k]
   if(name ~= nil) then
      url = "<A HREF=\"/lua/host_details.lua?host="..k.."\">"..name["name"].."</A>"
   else
      url = k
   end
   print("<tr><th>"..url.."</th><td class=\"text-right\"><div id=\""..string.gsub(k, '%.', '_').."\">" .. formatValue(v) .. "</div></td></tr>\n")
end
print("</table></td>\n")


print("</table>\n")
else
   print("No contacts for this host")
end


else
   print(page)
end
end


print("<script>\nvar contacts = " .. host["pkts.rcvd"] .. ";") 
print [[

setInterval(function() {
	  $.ajax({
		    type: 'GET',
		    url: '/lua/get_aggregated_host_info.lua',
		    data: { ifname: "]] print(ifname) print [[", name: "]] print(host_ip) print [[" },
		    success: function(content) {
			var rsp = jQuery.parseJSON(content);
			$('#last_seen').html(rsp.last_seen);
			$('#contacts').html(addCommas(rsp.num_contacts));
			
			if(contacts == rsp.num_contacts) {
			   $('#contacts_trend').html("<i class=icon-minus></i>");
			} else {
			   $('#contacts_trend').html("<i class=icon-arrow-up></i>");
			}
			contacts = rsp.num_contacts;

			for (var i = 0; i < rsp.contacts.length; i++) {
			   var key = '#'+rsp.contacts[i].key.replace(/\./g, '_');
			   $(key).html(addCommas(rsp.contacts[i].value));
			   $("#192.165.67.166").html('9');

			}
		     }
	           });
		 }, 3000);
</script>
		      ]]
dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
