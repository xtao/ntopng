--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

active_page = "if_stats"

-- First switch interfaces so the new cookie will have effect
ifname = interface.setActiveInterfaceId(tonumber(_GET["id"]))

if(ifname ~= nil) then
   key = "sessions." .. _SESSION["session"] .. ".ifname"
   ntop.setCache(key, ifname)	

   sendHTTPHeaderIfName('text/html', ifname, 3600)
   ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")
   dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

   print("<div class=\"alert alert-success\">The selected interface <b>" .. ifname)
   key = 'ntopng.prefs.'..ifname..'.name'
   custom_name = ntop.getCache(key)
   
   if((custom_name ~= nil) and (custom_name ~= "")) then
      print(" (".. custom_name ..")")
   end

   print("</b> is now active</div>")
   
   ntop.setCache('ntopng.prefs.'.._SESSION["user"]..'.iface', _GET["id"])

print [[


<script>
  window.setTimeout('window.location="/"; ', 3000);
</script>
]]


else
  sendHTTPHeader('text/html')
   ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")
   dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

  print("<div class=\"alert alert-error\"><img src=/img/warning.png> Error while switching interfaces</div>")
end

dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")