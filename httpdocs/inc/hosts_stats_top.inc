	
	       showPagination: true,
	       buttons: [ '<div class="btn-group"><button class="btn dropdown-toggle" data-toggle="dropdown">Filter Hosts<span class="caret"></span></button> <ul class="dropdown-menu"><li><a href="/lua/hosts_stats.lua">All Hosts</a></li><li><a href="/lua/hosts_stats.lua?mode=local">Local Only</a></li><li><a href="/lua/hosts_stats.lua?mode=remote">Remote Only</a></li></ul> </div>' ],
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
			        textAlign: 'center'
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
