--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"
require "top_talkers"

sendHTTPHeader('text/html')


print(getTopTalkers(getInterfaceId(ifname), ifname, _GET["mode"], _GET["epoch"]))