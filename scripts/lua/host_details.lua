ntop.dumpFile("./httpdocs/inc/header.inc")

ntop.dumpFile("./httpdocs/inc/menu.inc")

print [[


<ul class="breadcrumb">
  <li><A HREF=/hosts_stats.lua>Hosts</A> <span class="divider">/</span></li>
]]


print("<li>".._GET["host"].."</li>"	)

print [[
</ul>


Hello


]]
dofile "./scripts/lua/footer.inc.lua"
