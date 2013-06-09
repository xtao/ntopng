--
-- (C) 2013 - ntop.org
--

package.path = "./scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"

sendHTTPHeader('text/html')
ntop.dumpFile("./httpdocs/inc/header.inc")

active_page = "flows"
dofile "./scripts/lua/inc/menu.lua"

ntop.dumpFile("./httpdocs/inc/flows_stats.inc")
dofile "./scripts/lua/inc/footer.lua"
