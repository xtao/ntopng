--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.workingdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')

ntop.dumpFile("./httpdocs/inc/header.inc")
ntop.dumpFile("./httpdocs/inc/interface_stats.inc")
dofile "./scripts/lua/inc/footer.lua"
