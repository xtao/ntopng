--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.workingdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')

ntop.dumpFile(dirs.workingdir .. "/httpdocs/inc/header.inc")
ntop.dumpFile(dirs.workingdir .. "/httpdocs/inc/interface_stats.inc")
dofile dirs.workingdir .. "/scripts/lua/inc/footer.lua"
