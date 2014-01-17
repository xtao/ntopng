--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')
ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")

active_page = "alerts"
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

print [[
      <hr>
      <div id="table-alerts"></div>
	 <script>
	 $("#table-alerts").datatable({
			url: "/lua/get_alerts_data.lua",
	       showPagination: true,
	        title: "Queued Alerts",
	         columns: [
 {
         title: "Info",
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
         title: "Message",
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
