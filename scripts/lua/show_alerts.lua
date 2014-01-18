--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')
ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")

if(_GET["id_to_delete"] ~= nil) then
   ntop.deleteQueuedAlert(tonumber(_GET["id_to_delete"]))
end

active_page = "alerts"
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

print [[
      <hr>
      <div id="table-alerts"></div>
	 <script>
	 $("#table-alerts").datatable({
			url: "/lua/get_alerts_data.lua",
	       showPagination: true,

				   ]]

				   if(_GET["currentPage"] ~= nil) then print("currentPage: ".._GET["currentPage"]..",\n") end
				   if(_GET["perPage"] ~= nil) then print("perPage: ".._GET["perPage"]..",\n") end
print [[
	        title: "Queued Alerts",
	         columns: [
 {
         title: "Action",
     field: "column_key",
                 css: { 
		            textAlign: 'center'
		 }
	      },

 {
         title: "Date",
     field: "column_date",
                 css: { 
		            textAlign: 'center'
		 }
	      },
 {
         title: "Severity",
     field: "column_severity",
                 css: { 
		            textAlign: 'center'
		 }
	      },

 {
         title: "Type",
     field: "column_type",
                 css: { 
		            textAlign: 'center'
		 }
	      },

 {
         title: "Description",
     field: "column_msg",
                 css: { 
		            textAlign: 'left'
		 }
	      }
]
   });
   </script>
	      ]]



dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
