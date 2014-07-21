--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

ifstats = interface.getStats()
prefs = ntop.getPrefs()
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
      <li><a href="http://bugzilla.ntop.org/"><i class="fa fa-bug"></i> Report an Issue</a></li>
      <li class="divider"></li>
      <li><a href="/lua/index.lua"><i class="fa fa-dashboard"></i> Dashboard</a></li>
      <li><a href="/lua/logout.lua"><i class="fa fa-off"></i> Logout</a></li>
    </ul>
  </li>

   ]]

ifstats = interface.getStats()

if(ifstats.iface_sprobe) then
   url = "/lua/sflows_stats.lua"
else
   url = "/lua/flows_stats.lua"
end

if(active_page == "flows") then
   print ('<li class="active"><a href="'..url..'">Flows</a></li>')
else
   print ('<li><a href="'..url..'">Flows</a></li>')
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
      ]]
  if not (interface.isHistoricalInterface(ifstats.id)) then
	 print('<li><a href="/lua/top_hosts.lua"><i class="fa fa-trophy"></i> Top Hosts (Local)</a></li>')
  end

if(ifstats.iface_sprobe) then
   print('<li><a href="/lua/processes_stats.lua">Processes List</a></li>')
end

agg = interface.getNumAggregatedHosts()

if((agg ~= nil) and (agg > 0)) then
   print("<li><a href=\"/lua/aggregated_hosts_stats.lua\"><i class=\"fa fa-group\"></i> Aggregations</a></li>\n")
end

print [[
      <li class="divider"></li>
      <li><a href="/lua/hosts_interaction.lua">Interactions</a></li>
]]

if(ifstats.iface_sprobe) then
   print('<li><a href="/lua/sprobe.lua"><i class="fa fa-flag"></i> System Interactions</a></li>\n')
end


print [[
      <li><a href="/lua/hosts_flows_matrix.lua">Top Hosts Traffic</a></li>
   ]]

if(not(isLoopback(ifname))) then
   print [[
	    <li><a href="/lua/hosts_geomap.lua"><i class="fa fa-map-marker"></i> Geo Map</a></li>
	    <li><a href="/lua/hosts_treemap.lua"><i class="fa fa-sitemap"></i> Tree Map</a></li>
      ]]
end

print [[
      <li><a href="/lua/hosts_matrix.lua"><i class="fa fa-th-large"></i> Local Matrix</a></li>
    </ul>
  </li>

   ]]


-- Protocols

if(ifstats.aggregations_enabled and (not(ifstats.iface_sprobe))) then
if((ifstats["ndpi"]["EPP"] ~= nil) or (ifstats["ndpi"]["DNS"] ~= nil)) then

if active_page == "protocols_stats" then
  print [[ <li class="dropdown active"> ]]
else
  print [[ <li class="dropdown"> ]]
end
print [[
      <a class="dropdown-toggle" data-toggle="dropdown" href="#">Protocols <b class="caret"></b>
      </a>

    <ul class="dropdown-menu">
   ]]

if(ifstats["ndpi"]["EPP"] ~= nil) then
print [[



<li class="dropdown-submenu">
    <a tabindex="-1" href="#">EPP</a>
    <ul class="dropdown-menu">
   <li><a tabindex="-1" href="/lua/hosts_stats.lua?mode=local&protocol=EPP"> Hosts </a></li>
   <li><a tabindex="-1" href="/lua/protocols/epp_aggregations.lua?protocol=38&aggregation=1"> Server </a></li>
   <li><a tabindex="-1" href="/lua/protocols/epp_aggregations.lua?protocol=38&aggregation=4"> Registrar </a></li>
   <li><a tabindex="-1" href="/lua/protocols/epp_aggregations.lua?protocol=38&aggregation=2&tracked=1"> Existing Domains </a></li>
   <li><a tabindex="-1" href="/lua/protocols/epp_aggregations.lua?protocol=38&aggregation=2&tracked=0"> Unknown Domains </a></li>

  </ul>



   ]]
end


if(ifstats["ndpi"]["DNS"] ~= nil) then print('<li><A href="/lua/protocols/dns_aggregations.lua">DNS</A>') end

print [[
    </ul>
   </li>
   ]]
end
end



-- Interfaces
if(num_ifaces > 0) then
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


for k,v in pairs(names) do
    if(v == ifname) then
    print("<li")
    key = 'ntopng.prefs.'..v..'.name'
    custom_name = ntop.getCache(key)

    print(">")
    print("<a href=\"/lua/if_stats.lua?if_name="..v.."\" data-toggle=\"tooltip\" data-placement=\"left\" title=\"Current ")
    if (isPausedInterface(v)) then  print('and paused ') end
    print("interface\"> ")
    if(v == ifname) then print("<i class=\"fa fa-check\"></i> ") end
    if (isPausedInterface(v)) then  print('<i class="fa fa-pause"></i> ') end
    if((custom_name ~= nil) and (custom_name ~= "")) then
       print(custom_name)
    else
      print (v)
    end


    print("</a></li>")
    end
end


for k,v in pairs(names) do
    if(v ~= ifname) then
    print("<li")
    print("><a href=\"/lua/set_active_interface.lua?id="..k.."\" data-toggle=\"tooltip\" data-placement=\"left\" title=\"")
    if (isPausedInterface(v)) then  print('Paused interface') end
    print("\"> ")
    if (isPausedInterface(v)) then  print('<i class="fa fa-pause"></i> ') end
    key = 'ntopng.prefs.'..v..'.name'
    custom_name = ntop.getCache(key)

    if((custom_name ~= nil) and (custom_name ~= "")) then
       print(custom_name)
    else
      print (v)
    end

    print(' </a></li>')
    end

print [[
]]

end

-- Historical interface disable
if not (prefs.is_dump_flows_enabled) then
  print('<li> <a data-toggle="tooltip" data-placement="bottom" title="In order to enable this interface, you have to start ntopng with -F option." >Historical</a></li>')
end

print [[
</ul>
</li>
]]
end



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
      <li><a href="/lua/admin/prefs.lua"><i class="fa fa-flask"></i> Preferences</a></li>
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

