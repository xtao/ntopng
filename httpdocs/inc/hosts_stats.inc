

      <hr>



      <div id="table-hosts"></div>
	 <script>
	 $("#table-hosts").datatable({
	   url: "/get_hosts_data.lua",
	       showPagination: true,
	       title: "Hosts List",
	        columns: [
			     {
			     title: "IP Address",
				 field: "column_ip",
				 sortable: true,
	 	             css: { 
			        textAlign: 'left'
			     }
				 },
			     {
			     title: "Location",
				 field: "column_location",
				 sortable: false,
	 	             css: { 
			        textAlign: 'left'
			     }

				 },			     
			     {
			     title: "Symbolic Name",
				 field: "column_name",
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
			     title: "ASN",
				 field: "column_asn",
				 sortable: true,
	 	             css: { 
			        textAlign: 'center'
			     }

				 },
			     {
			     title: "Category",
				 field: "column_category",
				 sortable: true,
	 	             css: { 
			        textAlign: 'center'
			     }

				 },
			     {
			     title: "Breakdown",
				 field: "column_breakdown",
				 sortable: false,
	 	             css: { 
			        textAlign: 'center'
			     }
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


