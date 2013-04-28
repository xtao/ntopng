
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
				 field: "column_0",
				 sortable: true,
				 },
			     {
			     title: "Symbolic Name",
				 field: "column_1",
				 sortable: true,
				 },
			     {
			     title: "Traffic",
				 field: "column_2",

				 sortable: true,
	 	             css: { 
			        textAlign: 'right'
			     }
				 }
			     ]
	       });
       </script>







