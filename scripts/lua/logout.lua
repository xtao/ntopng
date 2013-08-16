--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeaderLogout('text/html')

ntop.deleteKey("sessions.".._SESSION["session"])
ntop.deleteKey("sessions.".._SESSION["session"]..".ifname")

print [[
 <meta http-equiv="refresh" content="1; URL=/">
<html>
<body>
 Logging out...
</body>
</html>

]]

