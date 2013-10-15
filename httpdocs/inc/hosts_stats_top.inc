

      <hr>



      <div id="table-hosts"></div>
	 <script>
	 $("#table-hosts").datatable({
	   url: "/lua/get_hosts_data.lua",
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
			     title: "VLAN",
				 field: "column_vlan",
				 sortable: true,
	 	             css: { 
			        textAlign: 'center'
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
			     title: "Name",
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
