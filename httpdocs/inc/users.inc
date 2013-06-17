

      <hr>


      <div id="table-users"></div>
	 <script>
	 $("#table-users").datatable({
	   url: "/lua/admin/get_users.lua",
	       showPagination: true,
	       title: "Users",
	        columns: [
			{
				title: "Username",
				field: "column_username",
				sortable: true,
	 	        	css: { 
					textAlign: 'left'
				}
			},
			{
				title: "Full Name",
				field: "column_full_name",
				sortable: true,
	 	        	css: { 
					textAlign: 'left'
				}

			},			     
			{
				title: "Group",
				field: "column_group",
				sortable: true,
	 	        	css: { 
					textAlign: 'center'
				}
			},
                        {
                        	title: "Edit",
                        	field: "column_edit",
                        	css: {
                        		textAlign: 'center'
                        	}
                       	},
		]
	 });
         </script>

