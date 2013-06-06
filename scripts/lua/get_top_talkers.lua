--
-- (C) 2013 - ntop.org
--

package.path = "./scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"
require "top_talkers"

tracked_host = _GET["host"]

ifname = _GET["if"]

print(getTopTalkers(_GET["if"]))

