ntop.dumpFile("./httpdocs/inc/header.inc")

ntop.dumpFile("./httpdocs/inc/menu.inc")

print [[


<ul class="breadcrumb">
  <li><A HREF=/flows_stats.lua>Flows</A> <span class="divider">/</span></li>
]]


print("<li>".._GET["label"].."</li>"	)

print [[
</ul>


Hello


]]
dofile "./scripts/lua/footer.inc.lua"
