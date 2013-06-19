--
-- (C) 2013 - ntop.org
--

package.path = "./scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"

sendHTTPHeader('text/html')

ntop.deleteKey("sessions.".._SESSION["session"])

print [[
 <meta http-equiv="refresh" content="1; URL=/">
]]

