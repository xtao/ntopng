--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeaderLogout('text/html')

ntop.delCache("sessions.".._SESSION["session"])
ntop.delCache("sessions.".._SESSION["session"]..".ifname")

-- io.write("Deleting ".."sessions.".._SESSION["session"].."\n")
-- io.write("Deleting ".."sessions.".._SESSION["session"]..".ifname\n")


print [[
 <meta http-equiv="refresh" content="1; URL=/">
<html>
<body>
 Logging out...
</body>
</html>

]]

