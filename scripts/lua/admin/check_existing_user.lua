--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"

sendHTTPHeader('application/json')

key = "ntopng.user.".. _GET["user"] ..".password"
existing = ntop.getCache(key)

if(existing ~= "") then
   valid = 0
else
   valid = 1
end

print('{ "valid" : ' .. valid ..', "user": "'.. _GET["user"]..'"}\n')