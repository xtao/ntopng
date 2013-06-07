--
-- (C) 2013 - ntop.org
--

ntop.dumpFile("./httpdocs/inc/header.inc")

active_page = "hosts"
dofile "./scripts/lua/menu.lua"

ntop.dumpFile("./httpdocs/inc/hosts_stats.inc")
dofile "./scripts/lua/footer.inc.lua"
