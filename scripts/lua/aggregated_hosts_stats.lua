--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")

active_page = "hosts"
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")


print [[
      <hr>
      <div id="table-hosts"></div>
	 <script>
	 $("#table-hosts").datatable({
					url: "/lua/get_hosts_data.lua?aggregated=1]]
					if(_GET["protocol"]) then print("&protocol=".._GET["protocol"]) end
					if(_GET["client"]) then print("&client=".._GET["client"]) end
print [[",
	       showPagination: true,
	       buttons: [ '<div class="btn-group"><button class="btn dropdown-toggle" data-toggle="dropdown">Aggregations<span class="caret"></span></button> <ul class="dropdown-menu">]]

print('<li><a href="/lua/aggregated_hosts_stats.lua">All</a></li>')

families = interface.getAggregationFamilies()
for key,v in pairs(families) do
   print('<li><a href="/lua/aggregated_hosts_stats.lua?protocol=' .. v..'">'..key..'</a></li>')
end 

print("</ul> </div>' ],\n")
print("title: \"Aggregations\",\n")

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/aggregated_hosts_stats_top.inc")

prefs = ntop.getPrefs()

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/aggregated_hosts_stats_bottom.inc")
dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
