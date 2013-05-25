ntop.dumpFile("./httpdocs/inc/header.inc")

dofile "./scripts/lua/host_menu.lua"

ntop.dumpFile("./httpdocs/inc/hosts_stats.inc")
dofile "./scripts/lua/footer.inc.lua"
