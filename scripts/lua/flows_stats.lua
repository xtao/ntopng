ntop.dumpFile("./httpdocs/inc/header.inc")

print [[
      <div class="masthead">
        <ul class="nav nav-pills pull-right">
          <li><a href="/">Home</a></li>
          <li class="active"><a href="/flows_stats.lua">Flows</a></li>
          <li><a href="/hosts_stats.lua">Hosts</a></li>
   ]]

dofile("./scripts/lua/search_host_box.lua")

print [[
  </ul>
        <h3 class="muted"><img src="/img/logo.png"></h3>
      </div>
   ]]

ntop.dumpFile("./httpdocs/inc/flows_stats.inc")
dofile "./scripts/lua/footer.inc.lua"
