--
-- (C) 2013 - ntop.org
--

package.path = "./scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"

sendHTTPHeader('text/html')

ntop.dumpFile("./httpdocs/inc/header.inc")

ntop.dumpFile("./httpdocs/inc/menu.inc")

print [[


<ul class="breadcrumb">
  <li><A HREF=/lua/flows_stats.lua>Flows</A> <span class="divider">/</span></li>
]]


print("<li>L4 Port: ".._GET["port"].."</li>"	)

print [[
</ul>


Hello


]]
dofile "./scripts/lua/inc/footer.lua"
