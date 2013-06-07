--
-- (C) 2013 - ntop.org
--

ntop.dumpFile("./httpdocs/inc/header.inc")

active_page = "flows"
dofile "./scripts/lua/menu.lua"

ntop.dumpFile("./httpdocs/inc/flows_stats.inc")
dofile "./scripts/lua/footer.inc.lua"
