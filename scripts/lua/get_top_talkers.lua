--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.workingdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "top_talkers"

sendHTTPHeader('text/html')

tracked_host = _GET["host"]

ifname = _GET["if"]

print(getTopTalkers(_GET["if"]))

