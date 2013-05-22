ntop.dumpFile("./httpdocs/inc/header.inc")

print [[
      <div class="masthead">
        <ul class="nav nav-pills pull-right">
          <li class="active"><a href="#">Home</a></li>
          <li><a href="/flows_stats.lua">Flows</a></li>
          <li><a href="/hosts_stats.lua">Hosts</a></li>
   ]]

dofile("./scripts/lua/search_host_box.lua")

print [[
  </ul>
        <h3 class="muted"><A href=http://www.ntop.org><img src="/img/logo.png"></A></h3>
      </div>
   ]]
ntop.dumpFile("./httpdocs/inc/index_top.inc")
dofile("./scripts/lua/sankey.lua")
ntop.dumpFile("./httpdocs/inc/index_bottom.inc")
dofile("./scripts/lua/footer.inc.lua")
