--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')

interface.find(ifname)
ifstats = interface.getStats()

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")

active_page = "home"
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

if(ifstats.stats_packets > 0) then
  ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/index_top.inc")
  dofile(dirs.installdir .. "/scripts/lua/inc/sankey.lua")
  ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/index_bottom.inc")
else
print("<div class=\"alert alert-warning\">No packet has been received yet on interface " .. ifname .. ".<p>Please wait and then reload this page in a few seconds.</div>")
end

  dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")