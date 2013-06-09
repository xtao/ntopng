--
-- (C) 2013 - ntop.org
--

print [[
      <div class="masthead">
        <ul class="nav nav-pills pull-right">
   ]]

if active_page == "home" then
  print [[ <li class="active"><a href="#">Home</a></li> ]]
else
  print [[ <li><a href="/">Home</a></li> ]]
end

if active_page == "flows" then
  print [[ <li class="active"><a href="#">Flows</a></li> ]]
else
  print [[ <li><a href="/lua/flows_stats.lua">Flows</a></li> ]]
end

if active_page == "hosts" then
  print [[ <li class="dropdown active"> ]]
else
  print [[ <li class="dropdown"> ]]
end

print [[
      <a class="dropdown-toggle" data-toggle="dropdown" href="#">
        Hosts <b class="caret"></b>
      </a>
    <ul class="dropdown-menu">
      <li><a href="/lua/hosts_stats.lua">Hosts List</a></li>
      <li><a href="/lua/hosts_matrix.lua">Local Matrix</a></li>
    </ul>
  </li>

   ]]

if active_page == "about" then
  print [[ <li class="active"><a href="#">About</a></li> ]]
else
  print [[ <li><a href="/lua/about.lua">About</a></li> ]]
end

dofile("./scripts/lua/inc/search_host_box.lua")

print [[
  </ul>
        <h3 class="muted"><A href=http://www.ntop.org><img src="/img/logo.png"></A></h3>
      </div>
   ]]

