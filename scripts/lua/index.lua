--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")

active_page = "home"
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/index_top.inc")
dofile(dirs.installdir .. "/scripts/lua/inc/sankey.lua")
ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/index_bottom.inc")
dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
