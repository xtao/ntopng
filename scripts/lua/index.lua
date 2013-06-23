--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.workingdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')

ntop.dumpFile(dir.workingdir .. "/httpdocs/inc/header.inc")

active_page = "home"
dofile dirs.workingdir .. "/scripts/lua/inc/menu.lua"

ntop.dumpFile(dir.workingdir .. "/httpdocs/inc/index_top.inc")
dofile(dirs.workingdir .. "/scripts/lua/inc/sankey.lua")
ntop.dumpFile(dir.workingdir .. "/httpdocs/inc/index_bottom.inc")
dofile(dirs.workingdir .. "/scripts/lua/inc/footer.lua")
