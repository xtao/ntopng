--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")

mode = _GET["mode"]
if(mode == nil) then mode = "all" end

active_page = "hosts"
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

print [[
      <hr>
      <div id="table-hosts"></div>
	 <script>
	 $("#table-hosts").datatable({
					title: "Active Processes",
					url: "/lua/get_processes_data.lua",
				  ]]

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/processes_stats.inc")
dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
