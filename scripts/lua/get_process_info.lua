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
if(page == nil) then page = "Protocols" end
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

pid_key = _GET["pid"]
name_key = _GET["name"]
if((pid_key == nil) and (name_key == nil))then
   print("<div class=\"alert alert-error\"><img src=/img/warning.png> Missing pid name</div>")
else

  if (pid_key ~= nil) then
   flows = interface.findPidFlows(tonumber(pid_key))
   err_label = "PID"
   err_val = pid_key
  elseif (name_key ~= nil) then
   flows = interface.findNameFlows(name_key)
   err = "Name"
   err_val = name_key
  end
   
   if(flows == nil) then
      print("<div class=\"alert alert-error\"><img src=/img/warning.png> Unknown "..err.." "..err_val..": no traffic detected for this process, or process terminated.</div>")
      else
   print [[
	    <div class="bs-docs-example">
            <div class="navbar">
	    <div class="navbar-inner">
	    <ul class="nav"> ]]
if(pid_key ~= nil)then
   print [[ <li><a href="#">Pid: ]] print(pid_key) print [[ </a></li>]]
elseif (name_key ~= nil)then
    print [[ <li><a href="#">Name: ]] print(name_key) print [[ </a></li>]]
end

if(page == "Protocols") then active=' class="active"' else active = "" end

if (pid_key ~= nil) then
  print('<li'..active..'><a href="?pid='.. pid_key ..'&page=Protocols">Protocols</a></li>\n')
  elseif (name_key ~= nil) then
   print('<li'..active..'><a href="?name='.. name_key ..'&page=Protocols">Protocols</a></li>\n')
  end

-- End Tab Menu

print('</ul>\n\t</div>\n\t</div>\n')


if(page == "Protocols") then

print [[
    <table class="table table-bordered table-striped">
      <tr><th class="text-center">
      L7 Protocols</th>
        <td><div class="pie-chart" id="topApps"></div></td>
      </th>
    </tr>
    <tr> 
      <th class="text-center">
        L4 Protocols
      </th>
      <td><div class="pie-chart" id="topL4"></div></td>
    </tr>]]

 print [[
      </table>
<script type='text/javascript'>
window.onload=function() {
   var refresh = 3000 /* ms */;
]]
if(pid_key ~= nil)then
   print [[ 
  do_pie("#topApps", '/lua/pid_stats.lua', { "pid": ]] print(pid_key) print [[, "mode": "l7"  }, "", refresh);
 do_pie("#topL4", '/lua/pid_stats.lua', { "pid": ]] print(pid_key) print [[, "mode": "l4"  }, "", refresh); 
  ]]
elseif (name_key ~= nil)then
    print [[ 
    do_pie("#topApps", '/lua/pid_stats.lua', { "name": "]] print(name_key) print [[", "mode": "l7"  }, "", refresh);
    do_pie("#topL4", '/lua/pid_stats.lua', { "name": "]] print(name_key) print [[", "mode": "l4"  }, "", refresh); ]]
end
print [[	    
}
</script>
]]
end
end
end

dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
