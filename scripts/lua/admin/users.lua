--
-- (C) 2013 - ntop.org
--

package.path = "./scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"

sendHTTPHeader('text/html')

ntop.dumpFile("./httpdocs/inc/header.inc")

active_page = "admin"
dofile "./scripts/lua/inc/menu.lua"

ntop.dumpFile("./httpdocs/inc/users.inc")
ntop.dumpFile("./httpdocs/inc/password_dialog.inc")
ntop.dumpFile("./httpdocs/inc/add_user_dialog.inc")
ntop.dumpFile("./httpdocs/inc/delete_user_dialog.inc")

dofile "./scripts/lua/inc/footer.lua"
