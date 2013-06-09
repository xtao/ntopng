--
-- (C) 2013 - ntop.org
--

package.path = "./scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"
require "top_talkers"

sendHTTPHeader('text/html')

print(getTopTalkers(_GET["if"], _GET["mode"], _GET["epoch"]))