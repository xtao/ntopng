
      <div class="masthead">
        <ul class="nav nav-pills pull-right">
          <li><a href="/">Home</a></li>
          <li class="active"><a href="/flows_stats.lua">Flows</a></li>
          <li><a href="/hosts_stats.lua">Hosts</a></li>
        </ul>
        <h3 class="muted"><img src="/img/logo.png"></h3>
      </div>

      <hr>



      <div id="table-hosts"></div>
	 <script>
	 $("#table-hosts").datatable({
	   url: "/get_flows_data.lua",
	       showPagination: true,
	       title: "Active Flows",
	        columns: [
			     {
			     title: "Info",
				 field: "column_key",
				 },
			     {
			     title: "Application",
				 field: "column_ndpi",
				 sortable: true,
				 },
			     {
			     title: "L4 Proto",
				 field: "column_proto_l4",
				 sortable: true,
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
			     title: "Bytes",
				 field: "column_bytes",
				 sortable: true,
				 }
			     ]
	       });
       </script>







