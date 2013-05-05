
      <div class="masthead">
        <ul class="nav nav-pills pull-right">
          <li><a href="/">Home</a></li>
          <li><a href="/flows_stats.lua">Flows</a></li>
          <li class="active"><a href="/hosts_stats.lua">Hosts</a></li>
        </ul>
        <h3 class="muted"><img src="/img/logo.png"></h3>
      </div>

      <hr>



      <div id="table-hosts"></div>
	 <script>
	 $("#table-hosts").datatable({
	   url: "/get_hosts_data.lua",
	       showPagination: true,
	       title: "Hosts",
	        columns: [
			     {
			     title: "IP Address",
				 field: "column_ip",
				 sortable: true,
				 },
			     {
			     title: "Symbolic Name",
				 field: "column_name",
				 sortable: true,
				 },
			     {
			     title: "Seen Since",
				 field: "column_since",
				 sortable: true,
				 },
			     {
			     title: "Traffic",
				 field: "column_traffic",
				 sortable: true,
	 	             css: { 
			        textAlign: 'right'
			     }
				 }
			     ]
	       });
       </script>







