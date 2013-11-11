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
					title: "Hosts List",
					url: "/lua/get_hosts_data.lua?mode=]] 
print(mode..'",')

if(mode == "all") then
   print('title: "All Hosts",\n')
elseif(mode == "local") then
   print('title: "Local Hosts",\n')
else
   print('title: "Remote Hosts",\n')
end

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/hosts_stats_top.inc")

prefs = ntop.getPrefs()

if(prefs.is_categorization_enabled) then
print [[
			     {
			     title: "Category",
				 field: "column_category",
				 sortable: true,
	 	             css: { 
			        textAlign: 'center'
			       }
			       },

		       ]]
end

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/hosts_stats_bottom.inc")
dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
