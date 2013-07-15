

      <hr>



      <div id="table-hosts"></div>
	 <script>
	 $("#table-hosts").datatable({
	   url: "/lua/get_hosts_data.lua?aggregated=1",
	       showPagination: true,
	       title: "Hosts List",
	        columns: [
			     {
			     title: "Name",
				 field: "column_ip",
				 sortable: true,
	 	             css: { 
			        textAlign: 'left'
			       }
			     },

			     {
			     title: "Seen Since",
				 field: "column_since",
				 sortable: true,
	 	             css: { 
			        textAlign: 'center'
 			       }
		   	     },
			     {
			     title: "Query Number",
				 field: "column_traffic",
				 sortable: true,
	 	             css: { 
			        textAlign: 'center'
 			       }
		   	     }
