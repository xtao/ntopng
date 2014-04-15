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


print [[
      <hr>
      <div id="table-hosts"></div>
	 <script>
	 $("#table-hosts").datatable({
					url: "/lua/get_hosts_data.lua?aggregated=1&protocol=38]] -- 38 == EPP
					if(_GET["client"]) then print("&client=".._GET["client"]) end
					if(_GET["aggregation"] ~= nil) then print("&aggregation=".._GET["aggregation"]) end
print [[",
	       showPagination: true,
	       buttons: [ '<div class="btn-group"><button class="btn dropdown-toggle" data-toggle="dropdown">Aggregations<span class="caret"></span></button> <ul class="dropdown-menu">]]

families = interface.getAggregationFamilies()
for key,v in pairs(families["families"]) do

   for key1,v1 in pairs(families["aggregations"]) do
      print('<li><a href="/lua/epp/aggregated_hosts_stats.lua?protocol=' .. v..'&aggregation='..v1..'">- '..key..' ('..key1..')</a></li>')
   end
end 

print("</ul> </div>' ],\n")

aggregation = _GET["aggregation"]
if(aggregation == nil) then aggregation = 1 end

aggregation = tonumber(aggregation)

if((aggregation == nil) or (aggregation == 1)) then
   aggregation = 1
   print("title: \"EPP Servers\",\n")
   elseif(aggregation == 2) then
   print("title: \"EPP Queried Domains\",\n")
   elseif(aggregation == 4) then
   print("title: \"EPP Registrars\",\n")
else
   print("title: \"Aggregations\",\n")
end


ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/aggregated_hosts_stats_top.inc")

prefs = ntop.getPrefs()

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/aggregated_hosts_stats_bottom.inc")
dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
