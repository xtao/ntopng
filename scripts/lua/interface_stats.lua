--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.workingdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')

ntop.dumpFile(dir.workingdir .. "/httpdocs/inc/header.inc")
ntop.dumpFile(dir.workingdir .. "/httpdocs/inc/interface_stats.inc")
dofile dirs.workingdir .. "/scripts/lua/inc/footer.lua"
