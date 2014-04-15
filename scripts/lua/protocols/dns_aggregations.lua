--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")

active_page = "hosts"
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

aggregation = _GET["aggregation"]
if(aggregation == nil) then aggregation = 1 end


print [[
      <hr>
      <div id="table-hosts"></div>
	 <script>
	 $("#table-hosts").datatable({
					url: "/lua/get_hosts_data.lua?aggregated=1&protocol=5]] -- 5 == DNS
					if(_GET["client"]) then print("&client=".._GET["client"]) end

print [[",
	       showPagination: true,
]]



print("title: \"DNS Queries\",\n")


ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/aggregated_hosts_stats_top.inc")

prefs = ntop.getPrefs()

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/aggregated_hosts_stats_bottom.inc")
dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
