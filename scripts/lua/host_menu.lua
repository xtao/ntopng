--
-- (C) 2013 - ntop.org
--

print [[
      <div class="masthead">
        <ul class="nav nav-pills pull-right">
           <li><a href="/">Home</a></li>
          <li><a href="/flows_stats.lua">Flows</a></li>
	 <li class="dropdown active">
    <a class="dropdown-toggle active" data-toggle="dropdown" href="#">
        Hosts <b class="caret"></b>
      </a>
    <ul class="dropdown-menu">
      <li><a href="/hosts_stats.lua">Hosts List</a></li>
      <li><a href="/hosts_matrix.lua">Local Matrix</a></li>
    </ul>
  </li>

   ]]
dofile("./scripts/lua/search_host_box.lua")

print [[
  </ul>
        <h3 class="muted"><A href=http://www.ntop.org><img src="/img/logo.png"></A></h3>
      </div>
   ]]

