--
-- (C) 2013 - ntop.org
--

print [[
      <div class="masthead">
        <ul class="nav nav-pills pull-right">
   ]]

if active_page == "home" or active_page == "about" then
  print [[ <li class="dropdown active"> ]]
else
  print [[ <li class="dropdown"> ]]
end

print [[
      <a class="dropdown-toggle" data-toggle="dropdown" href="#">
        Home <b class="caret"></b>
      </a>
    <ul class="dropdown-menu">
      <li><a href="/lua/about.lua"><i class="icon-question-sign"></i> About ntopng</a></li>
      <li><a href="http://blog.ntop.org/"><i class="icon-globe"></i> ntop Blog</a></li>
      <li class="divider"></li>
      <li><a href="/"><i class="icon-home"></i> Dashboard</a></li>
      <li><a href="/lua/logout.lua"><i class="icon-off"></i> Logout</a></li>
    </ul>
  </li></a>

   ]]

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
      <li><a href="/lua/hosts_treemap.lua">Hosts TreeMap</a></li>
      <li><a href="/lua/hosts_matrix.lua">Local Matrix</a></li>
    </ul>
  </li></a>

   ]]

if active_page == "admin" then
  print [[ <li class="dropdown active"> ]]
else
  print [[ <li class="dropdown"> ]]
end

print [[
      <a class="dropdown-toggle" data-toggle="dropdown" href="#">
        Admin <b class="caret"></b>
      </a>
    <ul class="dropdown-menu">
      <li><a href="/lua/admin/users.lua"><i class="icon-user"></i> Manage Users</a></li>
      <!--li><a href="/lua/admin/settings.lua">Settings</a></li-->
    </ul>
  </li>

   ]]

dofile(dirs.installdir .. "/scripts/lua/inc/search_host_box.lua")

print [[
  </ul>
        <h3 class="muted"><A href=http://www.ntop.org><img src="/img/logo.png"></A></h3>
      </div>
   ]]

