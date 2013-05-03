ntop.dumpFile("./httpdocs/inc/header.inc")

ntop.dumpFile("./httpdocs/inc/menu.inc")

page = _GET["page"]


print [[


<ul class="breadcrumb">
  <li><A HREF=/hosts_stats.lua>Hosts</A> <span class="divider">/</span></li>
]]


print("<li>".._GET["host"].."</li>\n"	)

print [[
</ul>


<div class="bs-docs-example">
            <div class="navbar">
              <div class="navbar-inner">
<ul class="nav">
]]

url="/host_details.lua?host=".._GET["host"]

if((page == "overview") or (page == nil)) then
  print("<li class=\"active\"><a href=\"#\">Host Overview</a></li>\n")
else
  print("<li><a href=\""..url.."&page=overview\">Host Overview</a></li>")
end

if(page == "flows") then
  print("<li class=\"active\"><a href=\"#\">Active Flows</a></li>\n")
else
  print("<li><a href=\""..url.."&page=flows\">Active Flows</a></li>")
end

if(page == "historical") then
  print("<li class=\"active\"><a href=\"#\">Historical Activity</a></li>\n")
else
  print("<li><a href=\""..url.."&page=historical\">Historical Activity</a></li>")
end

print [[
</ul>
</div>
</div>
</div>
   ]]

if((page == "overview") or (page == nil)) then
   ifname = _GET["interface"]
   if(ifname == nil) then	  
      ifname = "any"
   end

   interface.find(ifname)
   host = interface.getHostInfo( _GET["host"])
   print(host["name"])

elseif(page == "flows") then

print [[
      <div id="table-hosts"></div>
	 <script>
	 $("#table-hosts").datatable({
				  ]]
				  print("url: \"/get_flows_data.lua?host=" .. _GET["host"].."\",\n")


print [[
	       showPagination: true,
	       title: "Active Flows",
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
else
   print(page)
end


dofile "./scripts/lua/footer.inc.lua"
