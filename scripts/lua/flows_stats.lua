--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')
ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")

active_page = "flows"
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

ifname      = _GET["if"]
application = _GET["application"]

stats = interface.getNdpiStats()

print [[
      <hr>
      <div id="table-hosts"></div>
	 <script>
	 $("#table-hosts").datatable({
			url: "/lua/get_flows_data.lua]] 

if(application ~= nil) then
   print("?application="..application)
      end

print [[",
	       showPagination: true,
	       buttons: [ '<div class="btn-group"><button class="btn dropdown-toggle" data-toggle="dropdown">Applications<span class="caret"></span></button> <ul class="dropdown-menu">]]


--for key, value in pairs(stats["ndpi"]) do
for key, value in pairsByKeys(stats["ndpi"], asc) do
   print('<li><a href="/lua/flows_stats.lua?application=' .. key..'">'..key..'</a></li>')
end

print("</ul> </div>' ],")


ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/flows_stats_top.inc")

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

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/flows_stats_bottom.inc")
dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
