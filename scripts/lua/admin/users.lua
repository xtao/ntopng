--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"

sendHTTPHeader('text/html')

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")

active_page = "admin"
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/users.inc")
ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/password_dialog.inc")
ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/add_user_dialog.inc")
ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/delete_user_dialog.inc")

dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
