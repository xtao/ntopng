ntop.dumpFile("./httpdocs/inc/header.inc")

ntop.dumpFile("./httpdocs/inc/menu.inc")

page = _GET["page"]


print [[


<ul class="breadcrumb">
  <li><A HREF=/hosts_stats.lua>Hosts</A> <span class="divider">/</span></li>
]]


print("<li>".._GET["host"].."</li>\n"	)

print [[
</ul>


<div class="bs-docs-example">
            <div class="navbar">
              <div class="navbar-inner">
<ul class="nav">
]]

url="/host_details.lua?host=".._GET["host"]

if((page == "overview") or (page == nil)) then
  print("<li class=\"active\"><a href=\"#\">Host Overview</a></li>\n")
else
  print("<li><a href=\""..url.."&page=overview\">Host Overview</a></li>")
end

if(page == "flows") then
  print("<li class=\"active\"><a href=\"#\">Active Flows</a></li>\n")
else
  print("<li><a href=\""..url.."&page=flows\">Active Flows</a></li>")
end

if(page == "historical") then
  print("<li class=\"active\"><a href=\"#\">Historical Activity</a></li>\n")
else
  print("<li><a href=\""..url.."&page=historical\">Historical Activity</a></li>")
end

print [[
</ul>
</div>
            </div>
          </div>

Hello


]]
dofile "./scripts/lua/footer.inc.lua"
