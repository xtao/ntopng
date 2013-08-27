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
      <li><a href="/lua/about.lua"><i class="icon-question-sign"></i> About ntopng</a></li>
      <li><a href="http://blog.ntop.org/"><i class="icon-globe"></i> ntop Blog</a></li>
      <li class="divider"></li>
      <li><a href="/"><i class="icon-home"></i> Dashboard</a></li>
      <li><a href="/lua/logout.lua"><i class="icon-off"></i> Logout</a></li>
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
	 <li><a href="/lua/top_hosts.lua"><i class="icon-signal"></i> Top Hosts (Local)</a></li>
   ]]

if(interface.getNumAggregatedHosts() > 0) then
   print("<li><a href=\"/lua/aggregated_hosts_stats.lua\">Aggregation List</a></li>\n")
end

print [[
      <li class="divider"></li>
      <li><a href="/lua/hosts_geomap.lua"><i class="icon-map-marker"></i> Geo Map</a></li>
      <li><a href="/lua/hosts_treemap.lua"><i class="icon-th"></i> Tree Map</a></li>
      <li><a href="/lua/hosts_matrix.lua"><i class="icon-th-large"></i> Local Matrix</a></li>
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
    if(v ~= ifname) then print(" class=\"disabled\"") end
    print("><a href=/lua/if_stats.lua> " .. v .. " </a></li>")
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
    print("><a tabindex=\"-1\" href=\"/lua/set_active_interface.lua?id="..k.."\"> " .. v .. " </a></li>")
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
        Admin <b class="caret"></b>
      </a>
    <ul class="dropdown-menu">
      <li><a href="/lua/admin/users.lua"><i class="icon-user"></i> Manage Users</a></li>
      <!--li><a href="/lua/admin/settings.lua">Settings</a></li-->

]]


print [[
    </ul>
  </li>

   ]]

dofile(dirs.installdir .. "/scripts/lua/inc/search_host_box.lua")

print [[
  </ul>

        <h3 class="muted"><A href=http://www.ntop.org><img src="/img/logo.png"></A></h3>
      </div>

<script>
$(document).ready(function () { $("a").tooltip({ 'selector': '', 'placement': 'bottom'  });});
</script>
   ]]

