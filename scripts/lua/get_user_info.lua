--
-- (C) 2014 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "flow_utils"

sendHTTPHeader('text/html')

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")

page = _GET["page"]
if(page == nil) then page = "UserApps" end
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

user_key = _GET["user"]
host_key = _GET["host"]
application = _GET["application"]

if(user_key == nil) then
   print("<div class=\"alert alert-error\"><img src=/img/warning.png> Missing user name</div>")
else
   print [[
	    <div class="bs-docs-example">
            <div class="navbar">
	    <div class="navbar-inner">
	    <ul class="nav">
	    <li><a href="#">User: ]] print(user_key) if(host_key ~= nill) then print(" - "..host_key) end print [[  </a></li>
   ]]


if(page == "UserApps") then active=' class="active"' else active = "" end
print('<li'..active..'><a href="?user='.. user_key) if(host_key ~= nill) then print("&host="..host_key) end print('&page=UserApps">Applications</a></li>\n')

if(page == "UserProtocols") then active=' class="active"' else active = "" end
print('<li'..active..'><a href="?user='.. user_key) if(host_key ~= nill) then print("&host="..host_key) end print('&page=UserProtocols">Protocols</a></li>\n')

if(page == "Flows") then active=' class="active"' else active = "" end
print('<li'..active..'><a href="?user='.. user_key) if(host_key ~= nill) then print("&host="..host_key) end print('&page=Flows">Flow</a></li>\n')


print('</ul>\n\t</div>\n\t</div>\n')


if(page == "UserApps") then
print [[
    <table class="table table-bordered table-striped">
      <tr><th class="text-center">
      <h4>Top Applications</h4>
        <td><div class="pie-chart" id="topApps"></div></td>
      
      </th>
    </tr>]]

 print [[
      </table>
<script type='text/javascript'>
window.onload=function() {
   var refresh = 3000 /* ms */;
		    do_pie("#topApps", '/lua/user_stats.lua', { user: "]] print(user_key) print [[", mode: "apps" ]] 
if (host_key ~= nil) then print(", host: \""..host_key.."\"") end
print [[
 }, "", refresh);
}
</script>
]]

elseif(page == "UserProtocols") then

print [[
    <table class="table table-bordered table-striped">
      <tr><th class="text-center">
      <h4>Top L7 Protocols</h4>
        <td><div class="pie-chart" id="topL7"></div></td>
      </th>
    </tr>
    <tr><th class="text-center">
      <h4>Top L4 Protocols</h4>
        <td><div class="pie-chart" id="topL4"></div></td>
      
      </th>
    </tr>
    ]]

 print [[
      </table>
<script type='text/javascript'>
window.onload=function() {
   var refresh = 3000 /* ms */;
		    do_pie("#topL7", '/lua/user_stats.lua', { user: "]] print(user_key) print [[", mode: "l7" ]] 
if (host_key ~= nil) then print(", host: \""..host_key.."\"") end
print [[
 }, "", refresh);
		    do_pie("#topL4", '/lua/user_stats.lua', { user: "]] print(user_key) print [[", mode: "l4" ]] 
if (host_key ~= nil) then print(", host: \""..host_key.."\"") end
print [[
 }, "", refresh);
}
</script>
]]

elseif(page == "Flows") then

stats = interface.getNdpiStats()
num_param = 0

print [[
      <hr>
      <div id="table-hosts"></div>
   <script>
   $("#table-hosts").datatable({
      url: "/lua/get_flows_data.lua]] 
if(application ~= nil) then
   print("?application="..application)
   num_param = num_param + 1
end

if(user_key ~= nil) then
  if (num_param > 0) then
    print("&")
  else
    print("?")
  end
   print("user="..user_key)
   num_param = num_param + 1
end

if(host_key ~= nil) then
  if (num_param > 0) then
    print("&")
  else
    print("?")
  end
  print("host="..host_key)
  num_param = num_param + 1
end

print [[",
         showPagination: true,
         buttons: [ '<div class="btn-group"><button class="btn dropdown-toggle" data-toggle="dropdown">Applications<span class="caret"></span></button> <ul class="dropdown-menu">]]

-- print('<li><a href="/lua/get_user_info?user='.. pid_key) if(host_key ~= nill) then print("&host="..host_key) end print(">All Proto</a></li>")
for key, value in pairsByKeys(stats["ndpi"], asc) do
   class_active = ''
   if(key == application) then
      class_active = ' class="active"'
   end
   print('<li '..class_active..'><a href="/lua/get_user_info.lua?user='.. user_key) if(host_key ~= nill) then print("&host="..host_key) end print('&page=Flows&application=' .. key..'">'..key..'</a></li>')
end


print("</ul> </div>' ],\n")


ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/sflows_stats_top.inc")

prefs = ntop.getPrefs()

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/sflows_stats_bottom.inc")




end -- If page



end



dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
