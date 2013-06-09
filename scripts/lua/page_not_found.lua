--
-- (C) 2013 - ntop.org
--

package.path = "./scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"

sendHTTPHeader('text/html')

ntop.dumpFile("./httpdocs/inc/header.inc")
ntop.dumpFile("./httpdocs/inc/menu.inc")

print('<div class="alert alert-error"><img src=/img/warning.png> Page not found</div>')

print ("<center><H4>Unable to find URL <i>")

print(_GET["url"])

print("</i></center></H4>\n")

dofile "./scripts/lua/inc/footer.lua"


