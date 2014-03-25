--
-- (C) 2014 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "graph_utils"


page        = _GET["page"]
hosts_ip     = _GET["hosts"]


-- Default values
if(page == nil) then 
  page = "overview"
end

active_traffic = true
active_packets = false
active_ndpi = false
show_aggregation = true

active_page = "hosts"
sendHTTPHeader('text/html')
ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")


if(hosts_ip == nil) then
   print("<div class=\"alert alert-error\"><img src=/img/warning.png> Hosts parameter is missing (internal error ?)</div>")
   return
end


print [[
<div class="bs-docs-example">
  <div class="navbar">
    <div class="navbar-inner">
<ul class="nav">
]]

url="/lua/hosts_comparison.lua?hosts="..hosts_ip

hosts_ip_tab_name = string.gsub(hosts_ip, ',', " <i class=\"fa fa-exchange fa-lg\"></i> ")

print("<li><a href=\"#\">Hosts: "..hosts_ip_tab_name.." </a></li>\n")

if((page == "overview") or (page == nil)) then
  print("<li class=\"active\"><a href=\"#\"><i class=\"fa fa-home fa-lg\"></i></a></li>\n")
else
  print("<li><a href=\""..url.."&page=overview\"><i class=\"fa fa-home fa-lg\"></i></a></li>")
end

if(page == "traffic") then
   print("<li class=\"active\"><a href=\"#\">Traffic</a></li>\n")
else
   if(active_traffic) then
      print("<li><a href=\""..url.."&page=traffic\">Traffic</a></li>")
   end
end

if(page == "packets") then
   print("<li class=\"active\"><a href=\"#\">Packets</a></li>\n")
else
   if(active_packets) then
      print("<li><a href=\""..url.."&page=packets\">Packets</a></li>")
   end
end

if(page == "ndpi") then
  print("<li class=\"active\"><a href=\"#\">Protocols</a></li>\n")
else
   if(active_ndpi) then
      print("<li><a href=\""..url.."&page=ndpi\">Protocols</a></li>")
   end
end

print [[
</ul>
</div>
</div>
</div>
   ]]

-- =========================== Tab Menu =================

if (page == "overview") then


if(show_aggregation) then
   print [[
<div class="btn-group">
  <button class="btn btn-small dropdown-toggle" data-toggle="dropdown">Aggregation <span class="caret"></span></button>
  <ul class="dropdown-menu">
]]

print('<li><a  href="'..url .. '&aggregation=ndpi">'.. "Application" ..'</a></li>\n')
print('<li><a  href="'..url .. '&aggregation=l4proto">'.. "Proto L4" ..'</a></li>\n')
print('<li><a  href="'..url .. '&aggregation=port">'.. "Port" ..'</a></li>\n')
print [[
  </ul>
</div><!-- /btn-group -->


]]



print('&nbsp;Refresh:  <div class="btn-group">\n')
 print[[
 <button id="graph_refresh" class="btn btn-small">
    <i rel="tooltip" data-toggle="tooltip" data-placement="top" data-original-title="Refresh graph" class="icon-refresh"></i></button>")
]]
print [[
</div>
</div>
<br/>
]]

print[[
<script>
   $("#graph_refresh").click(function() {
    sankey();
  });

  $(window).load(function() 
  {
   // disabled graph interval
   clearInterval(sankey_interval);
  });  
</script>

]]
end -- End if(show_aggregation)

-- =========================== Aggregation Menu =================
print("<center>")
print("<div class=\"row-fluid\">")
print("  <div>")
dofile(dirs.installdir .. "/scripts/lua/inc/sankey.lua")
print("  </div>")

print("</div>")
print("</center><br/>")


elseif(page == "traffic") then

if(show_aggregation) then
   print [[
<div class="btn-group">
  <button class="btn btn-small dropdown-toggle" data-toggle="dropdown">Aggregation <span class="caret"></span></button>
  <ul class="dropdown-menu">
]]

print('<li><a  href="'..url .. '&page=traffic'..'&aggregation=ndpi">'.. "Application" ..'</a></li>\n')
print('<li><a  href="'..url .. '&page=traffic'..'&aggregation=l4proto">'.. "Proto L4" ..'</a></li>\n')
print('<li><a  href="'..url .. '&page=traffic'..'&aggregation=port">'.. "Port" ..'</a></li>\n')
print [[
  </ul>
</div><!-- /btn-group -->


]]



print('&nbsp;Refresh:  <div class="btn-group">\n')
 print[[
 <button id="graph_refresh" class="btn btn-small">
    <i rel="tooltip" data-toggle="tooltip" data-placement="top" data-original-title="Refresh graph" class="icon-refresh"></i></button>")
]]
print [[
</div>
</div>
<br/>
]]

print[[
<script>
   $("#graph_refresh").click(function() {
    bubble();
  });

  $(window).load(function() 
  {
   // disabled graph interval
   clearInterval(bubble_interval);
  });  
</script>

]]
end -- End if(show_aggregation)

-- =========================== Aggregation Menu =================
print("<center>")
print("<div class=\"row-fluid\">")
print("  <div>")
dofile(dirs.installdir .. "/scripts/lua/inc/bubblechart.lua")
print("  </div>")
print("</div>")
print("</center>")



elseif(page == "packets") then


elseif(page == "ndpi") then


end -- End if page == ...

dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
