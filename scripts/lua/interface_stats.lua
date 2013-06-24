--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")
ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/interface_stats.inc")
dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
