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
if(page == nil) then page = "PidProtocols" end
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

pid_key = _GET["pid"]
if(pid_key == nil) then
   print("<div class=\"alert alert-error\"><img src=/img/warning.png> Missing pid name</div>")
else
   print [[
	    <div class="bs-docs-example">
            <div class="navbar">
	    <div class="navbar-inner">
	    <ul class="nav">
	    <li><a href="#">Pid: ]] print(pid_key) print [[ </a></li>
   ]]
end

if(page == "PidProtocols") then active=' class="active"' else active = "" end
print('<li'..active..'><a href="?pid='.. pid_key ..'&page=PidProtocols">L7 Protocols</a></li>\n')


print('</ul>\n\t</div>\n\t</div>\n')

if(page == "PidProtocols") then
print [[
<h4>Application Protocols</h4>
<div class="pie-chart" id="topApps"></div>

<script type='text/javascript'>
window.onload=function() {
   var refresh = 3000 /* ms */;
		    do_pie("#topApps", '/lua/pid_stats.lua', { "pid": ]] print(pid_key) print [[, "mode": "l7"  }, "", refresh);
}
</script>
]]

end

dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
