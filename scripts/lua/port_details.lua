--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.workingdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')

ntop.dumpFile(dirs.workingdir .. "/httpdocs/inc/header.inc")

dofile(dirs.workingdir .. "/scripts/lua/inc/menu.lua")

print [[


<ul class="breadcrumb">
  <li><A HREF=/lua/flows_stats.lua>Flows</A> <span class="divider">/</span></li>
]]


print("<li>L4 Port: ".._GET["port"].."</li></ul>")

print [[
      <div id="table-hosts"></div>
	 <script>
	 $("#table-hosts").datatable({
				  ]]
				  print("url: \"/lua/get_flows_data.lua?port=" .. _GET["port"].."\",\n")


print [[
	       showPagination: true,
	       title: "Active Flows on Port ]] print(_GET["port"]) print [[",
	        columns: [
			     {
			     title: "Info",
				 field: "column_key",
	 	             css: {
			        textAlign: 'center'
			     }
				 },
			     {
			     title: "Application",
				 field: "column_ndpi",
				 sortable: true,
	 	             css: {
			        textAlign: 'center'
			     }
				 },
			     {
			     title: "L4 Proto",
				 field: "column_proto_l4",
				 sortable: true,
	 	             css: {
			        textAlign: 'center'
			     }
				 },
			     {
			     title: "Client",
				 field: "column_client",
				 sortable: true,
				 },
			     {
			     title: "Server",
				 field: "column_server",
				 sortable: true,
				 },
			     {
			     title: "Duration",
				 field: "column_duration",
				 sortable: true,
	 	             css: {
			        textAlign: 'right'
			       }
			       },
			     {
			     title: "Bytes",
				 field: "column_bytes",
				 sortable: true,
	 	             css: {
			        textAlign: 'right'
			     }

				 }
			     ]
	       });
       </script>

   ]]


dofile dirs.workingdir .. "/scripts/lua/inc/footer.lua"
