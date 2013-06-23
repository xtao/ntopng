--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.workingdir .. "/scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"

sendHTTPHeader('text/html')

ntop.dumpFile(dirs.workingdir .. "/httpdocs/inc/header.inc")

active_page = "admin"
dofile(dirs.workingdir .. "/scripts/lua/inc/menu.lua")

ntop.dumpFile(dirs.workingdir .. "/httpdocs/inc/users.inc")
ntop.dumpFile(dirs.workingdir .. "/httpdocs/inc/password_dialog.inc")
ntop.dumpFile(dirs.workingdir .. "/httpdocs/inc/add_user_dialog.inc")
ntop.dumpFile(dirs.workingdir .. "/httpdocs/inc/delete_user_dialog.inc")

dofile(dirs.workingdir .. "/scripts/lua/inc/footer.lua")
