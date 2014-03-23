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
if(user_key == nil) then
   print("<div class=\"alert alert-error\"><img src=/img/warning.png> Missing user name</div>")
else
   print [[
	    <div class="bs-docs-example">
            <div class="navbar">
	    <div class="navbar-inner">
	    <ul class="nav">
	    <li><a href="#">User: ]] print(user_key) print [[ </a></li>
   ]]


if(page == "UserApps") then active=' class="active"' else active = "" end
print('<li'..active..'><a href="?user='.. user_key ..'&page=UserApps">Applications</a></li>\n')

if(page == "UserProtocols") then active=' class="active"' else active = "" end
print('<li'..active..'><a href="?user='.. user_key ..'&page=UserProtocols">L7 Protocols</a></li>\n')


print('</ul>\n\t</div>\n\t</div>\n')


if(page == "UserApps") then
print [[
<h4>Top Applications</h4>
<div class="pie-chart" id="topApps"></div>

<script type='text/javascript'>
window.onload=function() {
   var refresh = 3000 /* ms */;
		    do_pie("#topApps", '/lua/user_stats.lua', { user: "]] print(user_key) print [[", mode: "apps"  }, "", refresh);
}
</script>
]]

elseif(page == "UserProtocols") then

print [[
<h4>Top Application Protocols</h4>
<div class="pie-chart" id="topL7"></div>

<script type='text/javascript'>
window.onload=function() {
   var refresh = 3000 /* ms */;
		    do_pie("#topL7", '/lua/user_stats.lua', { user: "]] print(user_key) print [[", mode: "l7" }, "", refresh);
}
</script>
]]

end



end



dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
