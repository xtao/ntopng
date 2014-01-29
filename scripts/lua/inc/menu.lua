--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

names = interface.getIfNames()
num_ifaces = 0
for k,v in pairs(names) do num_ifaces = num_ifaces+1 end

print [[
      <div class="masthead">

        <ul class="nav nav-pills pull-right">
   ]]

interface.find(ifname)

if active_page == "home" or active_page == "about" then
  print [[ <li class="dropdown active"> ]]
else
  print [[ <li class="dropdown"> ]]
end

print [[


      <a class="dropdown-toggle" data-toggle="dropdown" href="#">
        Home <b class="caret"></b>
      </a>
    <ul class="dropdown-menu">
      <li><a href="/lua/about.lua"><i class="fa fa-question-circle"></i> About ntopng</a></li>
      <li><a href="http://blog.ntop.org/"><i class="fa fa-globe"></i> ntop Blog</a></li>
      <li class="divider"></li>
      <li><a href="/lua/index.lua"><i class="fa fa-dashboard"></i> Dashboard</a></li>
      <li><a href="/lua/logout.lua"><i class="fa fa-off"></i> Logout</a></li>
    </ul>
  </li>

   ]]

if active_page == "flows" then
  print [[ <li class="active"><a href="/lua/flows_stats.lua">Flows</a></li> ]]
else
  print [[ <li><a href="/lua/flows_stats.lua">Flows</a></li> ]]
end

if active_page == "hosts" then
  print [[ <li class="dropdown active"> ]]
else
  print [[ <li class="dropdown"> ]]
end

print [[
      <a class="dropdown-toggle" data-toggle="dropdown" href="#">
        Hosts <b class="caret"></b>
      </a>
    <ul class="dropdown-menu">
      <li><a href="/lua/hosts_stats.lua">Hosts List</a></li>
	 <li><a href="/lua/top_hosts.lua"><i class="fa fa-trophy"></i> Top Hosts (Local)</a></li>
   ]]

if(interface.getNumAggregatedHosts() > 0) then
   print("<li><a href=\"/lua/aggregated_hosts_stats.lua\"><i class=\"fa fa-group\"></i> Aggregations</a></li>\n")
end

print [[
      <li class="divider"></li>
      <li><a href="/lua/hosts_interaction.lua">Interactions</a></li>
      <li><a href="/lua/hosts_flows_matrix.lua">Top Hosts Traffic</a></li>
      <li><a href="/lua/hosts_geomap.lua"><i class="fa fa-map-marker"></i> Geo Map</a></li>
      <li><a href="/lua/hosts_treemap.lua"><i class="fa fa-sitemap"></i> Tree Map</a></li>
      <li><a href="/lua/hosts_matrix.lua"><i class="fa fa-th-large"></i> Local Matrix</a></li>
    </ul>
  </li>

   ]]

-- Interfaces
if active_page == "if_stats" then
  print [[ <li class="dropdown active"> ]]
else
  print [[ <li class="dropdown"> ]]
end

print [[
      <a class="dropdown-toggle" data-toggle="dropdown" href="#">Interfaces <b class="caret"></b>
      </a>
      <ul class="dropdown-menu">
]]

if(num_ifaces == 1) then
print [[
    <li class=disabled><a href="#" data-toggle="tooltip" data-original-title="You can specify multiple interfaces by repeating the -i &lt;iface&gt; CLI option" >
    <small>Available Interfaces</small></a></li>
   ]]
else
print('<li class=disabled><a href="#"><small>Available Interfaces</small></a></li>')
end

for k,v in pairs(names) do
    print("<li");
    key = 'ntopng.prefs.'..v..'.name'
    custom_name = ntop.getCache(key)

    if(v ~= ifname) then print(" class=\"disabled\"") end
    print(">")
    print("<a href=/lua/if_stats.lua> ")
    if(v == ifname) then print("<i class=\"fa fa-check\"></i> ") end
    print(v)

    if((custom_name ~= nil) and (custom_name ~= "")) then
       print(" (".. custom_name ..")")
    end
    print("</a></li>")
end



if(num_ifaces > 1) then

print [[
<li class="divider"></li>
<li class="dropdown-submenu">
    <a tabindex="-1" href="#">Switch Interfaces</a>
    <ul class="dropdown-menu">
]]

for k,v in pairs(names) do
    print("<li");
    if(v == ifname) then print(" class=\"disabled\"") end
    print("><a tabindex=\"-1\" href=\"/lua/set_active_interface.lua?id="..k.."\"> " .. v)
    key = 'ntopng.prefs.'..v..'.name'
    custom_name = ntop.getCache(key)

    if((custom_name ~= nil) and (custom_name ~= "")) then
       print(" (".. custom_name ..")")
    end

    print(" </a></li>")
end

print [[
    </ul>
  </li>
]]

end


print [[
</ul>
</li>
]]

-- Admin
if active_page == "admin" then
  print [[ <li class="dropdown active"> ]]
else
  print [[ <li class="dropdown"> ]]
end

print [[
      <a class="dropdown-toggle" data-toggle="dropdown" href="#">
        <i class="fa fa-cog fa-lg"></i> <b class="caret"></b>
      </a>
    <ul class="dropdown-menu">
      <li><a href="/lua/admin/users.lua"><i class="fa fa-user"></i> Manage Users</a></li>
      <!--li><a href="/lua/admin/settings.lua">Settings</a></li-->
      <li class="divider"></li>
      <li><a href="/lua/export_data.lua"><i class="fa fa-share"></i> Export Data</a></li>
    </ul>
    </li>
   ]]

if(ntop.getNumQueuedAlerts() > 0) then
print [[
<li>
<a  href="/lua/show_alerts.lua">
<i class="fa fa-warning fa-lg" style="color: #B94A48;"></i>
</a>
</li>
   ]]
end


dofile(dirs.installdir .. "/scripts/lua/inc/search_host_box.lua")

function file_exists(name)
   local f=io.open(name,"r")
   if f~=nil then io.close(f) return true else return false end
end

if(file_exists(dirs.installdir .. "/httpdocs/img/custom_logo.jpg")) then
   logo_path = "/img/custom_logo.jpg"
else
   logo_path = "/img/logo.png"
end

print ("</ul>\n<h3 class=\"muted\"><A href=http://www.ntop.org><img src=\""..logo_path.."\"></A></h3>\n</div>\n")

