--
-- (C) 2013 - ntop.org
--

ntop.dumpFile("./httpdocs/inc/header.inc")

active_page = "home"
dofile "./scripts/lua/menu.lua"

ntop.dumpFile("./httpdocs/inc/index_top.inc")
dofile("./scripts/lua/sankey.lua")
ntop.dumpFile("./httpdocs/inc/index_bottom.inc")
dofile("./scripts/lua/footer.inc.lua")
