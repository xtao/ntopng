ntop.dumpFile("./httpdocs/inc/header.inc")
ntop.dumpFile("./httpdocs/inc/menu.inc")

print ("<center><img src=/img/warning.png><H3>Unable to find URL <i>")

print(_GET["url"])

print("</i></H3></center>\n")

dofile "./scripts/lua/footer.inc.lua"


