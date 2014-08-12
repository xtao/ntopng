--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path
if ( (dirs.scriptdir ~= nil) and (dirs.scriptdir ~= "")) then package.path = dirs.scriptdir .. "/lua/modules/?.lua;" .. package.path end

require "lua_utils"

active_page = "if_stats"

-- First switch interfaces so the new cookie will have effect
ifname = interface.setActiveInterfaceId(tonumber(_GET["id"]))

if((ifname ~= nil) and (_SESSION["session"] ~= nil)) then
   key = getRedisPrefix("ntopng.prefs") .. ".ifname"
   ntop.setCache(key, ifname)

   sendHTTPHeaderIfName('text/html', ifname, 3600)
   ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")
   dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

   print("<div class=\"alert alert-success\">The selected interface <b>" .. getHumanReadableInterfaceName(_GET["id"]))

   print("</b> is now active</div>")

   ntop.setCache(getRedisPrefix("ntopng.prefs")..'.iface', _GET["id"])

print [[
<script>
  window.setTimeout('window.location="/"; ', 3000);
</script>
]]


else
  sendHTTPHeader('text/html')
   ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")
   dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

  print("<div class=\"alert alert-danger\"><img src=/img/warning.png> Error while switching interfaces</div>")
if(_SESSION["session"] == nil) then
  print("<div class=\"alert alert-danger\"><img src=/img/warning.png> Empty session</div>")
end

end

dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")