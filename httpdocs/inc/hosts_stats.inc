

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


