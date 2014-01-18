--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')
ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")

if(_GET["id_to_delete"] ~= nil) then
   if(_GET["id_to_delete"] == "all") then
      ntop.flushAllQueuedAlerts()
      print("")
   else
      ntop.deleteQueuedAlert(tonumber(_GET["id_to_delete"]))
   end
end

active_page = "alerts"
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

print(_GET["id_to_delete"])

print [[
      <hr>
      <div id="table-alerts"></div>
	 <script>
	 $("#table-alerts").datatable({
			url: "/lua/get_alerts_data.lua",
	       showPagination: true,
]]

if(_GET["currentPage"] ~= nil) then print("currentPage: ".._GET["currentPage"]..",\n") end
if(_GET["perPage"] ~= nil)     then print("perPage: ".._GET["perPage"]..",\n") end

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


if(ntop.getNumQueuedAlerts() > 0) then
   print [[
	    <form class=form-inline style="margin-bottom: 0px;" method=get action="#"><input type=hidden name=id_to_delete value="all"><button class="btn btn-mini" type="submit"><i type="submit" class="fa fa-trash-o"></i> Purge All Alarms</button></form>
      ]]
end

dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
