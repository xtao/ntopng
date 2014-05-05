--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")

protocol = _GET["protocol"]

mode = _GET["mode"]
if(mode == nil) then mode = "all" end

active_page = "hosts"
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")


prefs = ntop.getPrefs()

if(prefs.is_categorization_enabled) then print ()end 


print [[
      <hr>
      <div id="table-hosts"></div>
	 <script>
	 var url_update = "/lua/get_hosts_data.lua?mode=]]
print(mode)

if(protocol ~= nil) then
   print('&protocol='..protocol)
end

print [[";
   var url_update_all = url_update + "]]
print('&all=1";')

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/hosts_stats_id.inc") 
-- Set the flow table option
if(prefs.is_categorization_enabled) then print ('flow_rows_option["categorization"] = true;') end

if(prefs.is_httpbl_enabled) then print ('flow_rows_option["httpb"] = true;') end 
print [[
	 host_rows_option["ip"] = true;
	 $("#table-hosts").datatable({ 
	 		title: "Hosts List",
			url: url_update ,
	 ]]

if(protocol == nil) then protocol = "" end

if(mode == "all") then
   print('title: "All '..protocol..' Hosts",\n')
elseif(mode == "local") then
   print('title: "Local '..protocol..' Hosts",\n')
else
   print('title: "Remote '..protocol..' Hosts",\n')
end
print [[			
      	 rowCallback: function ( row ) { return host_table_setID(row); },
	       showPagination: true,
	       buttons: [ '<div class="btn-group"><button class="btn dropdown-toggle" data-toggle="dropdown">Filter Hosts<span class="caret"></span></button> <ul class="dropdown-menu"><li><a href="/lua/hosts_stats.lua">All Hosts</a></li><li><a href="/lua/hosts_stats.lua?mode=local">Local Only</a></li><li><a href="/lua/hosts_stats.lua?mode=remote">Remote Only</a></li></ul> </div>' ],
	        columns: [
	        	{
	        		title: "Key",
         			field: "key",
         			hidden: true,
         			css: { 
              textAlign: 'center'
           }
         		},
         		{
			     title: "IP Address",
				 field: "column_ip",
				 sortable: true,
	 	             css: {
			        textAlign: 'left'
			     }
				 },
			     {
			  ]]

ifstats = interface.getStats()

if(ifstats.iface_sprobe) then
   print('title: "Source Id",\n')
else
   print('title: "VLAN",\n')
end

print [[
				 field: "column_vlan",
				 sortable: true,
	 	             css: {
			        textAlign: 'center'
			     }

				 },
]]

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/hosts_stats_top.inc")



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

if(prefs.is_httpbl_enabled) then
print [[
			     {
			     title: "HTTP:BL",
				 field: "column_httpbl",
				 sortable: true,
	 	             css: {
			        textAlign: 'center'
			       }
			       },

		       ]]
end



ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/hosts_stats_bottom.inc")
dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
