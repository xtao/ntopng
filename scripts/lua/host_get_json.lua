--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

host_info = urt2hostinfo(_GET)

if(host_info["host"] == nil) then
   sendHTTPHeader('text/html')
   ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")
   dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")
   print("<div class=\"alert alert-error\"><img src=/img/warning.png> Host parameter is missing (internal error ?)</div>")
   return
end

interface.find(ifname)
host = interface.getHostInfo(host_info["host"], host_info["vlan"])

if(host == nil) then
   sendHTTPHeader('text/html')
   ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")
   dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")
   print("<div class=\"alert alert-error\"><img src=/img/warning.png> Host ".. host_info["host"] .. " Vlan" ..host_info["vlan"].." cannot be found (expired ?)</div>")
   return
else
   sendHTTPHeader('application/json')
   print(host["json"])
end