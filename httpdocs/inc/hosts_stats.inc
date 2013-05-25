

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
				 },
			     {
			     title: "Location",
				 field: "column_location",
				 sortable: false,
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
			     title: "ASN",
				 field: "column_asn",
				 sortable: true,
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


