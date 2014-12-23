--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"
require "top_talkers"

sendHTTPHeader('text/html; charset=iso-8859-1')

top_talkers = getTopTalkers(getInterfaceId(ifname), ifname, _GET["epoch"])
mod = require("top_scripts.top_talkers")
if (type(mod) == type(true)) then
  print("[ ]\n")
end
print(mod.getTopFromJSON(top_talkers))
