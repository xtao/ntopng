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
interface.find(ifname)
flows = interface.findUserFlows(user_key)

if(flows == nil) then
   print("<div class=\"alert alert-error\"><img src=/img/warning.png> No flow can be found for user " .. user_key .. ". (expired ?)</div>")
else
   print("<table class=\"table table-bordered\">\n")
   num = 0
   for k,v in pairs(flows) do num = num + 1 end

   print("<tr><th width=30%>Flows Found</th><td colspan=2>" .. num.. "</td></tr>\n")

   print("</table>\n")
end
end



dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
