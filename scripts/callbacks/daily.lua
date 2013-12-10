--
-- (C) 2013 - ntop.org
--


dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

local debug = false
local delete_keys = true

begin = os.clock()
t = os.time()-86400

if((_GET ~= nil) and (_GET["debug"] ~= nil)) then
   debug = true
   t = t + 86400
end

if(debug) then sendHTTPHeader('text/plain') end

when = os.date("%y%m%d", t)

ifnames = interface.getIfNames()
for _,_ifname in pairs(ifnames) do
   interface.find(purifyInterfaceName(_ifname))
   interface.flushHostContacts()
end

ntop.dumpDailyStats(when)

-- redis-cli KEYS "131129|*" | xargs redis-cli DEL
