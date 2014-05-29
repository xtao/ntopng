--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"

sendHTTPHeader('text/html')

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")

active_page = "admin"
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

print [[
   <h2>About ntopng</h2>
   <table class="table table-striped">
   ]]

toggleTableButton("RRDs For Local Hosts", 
		  "Toggle the creation of RRDs for local hosts. Turn it off to save storage space.",
		  "On", "Off", "toggle_local", "ntopng.prefs.host_rrd_creation")

toggleTableButton("nDPI RRDs For Local Hosts",
		  "Toggle the creation of nDPI RRDs for local hosts. Enable their creation allows you to keep application protocol statistics at the cost of using more disk space.",
		  "On", "Off", "toggle_local_ndpi", "ntopng.prefs.host_ndpi_rrd_creation")

print [[
   </table>

<script>
	 $('.btn-toggle').click(function() {
	      $(this).find('.btn').toggleClass('active');  
    
		if ($(this).find('.btn-primary').size()>0) {
		   $(this).find('.btn').toggleClass('btn-primary');
		}

		if ($(this).find('.btn-danger').size()>0) {
		    $(this).find('.btn').toggleClass('btn-danger');
	        }
		if ($(this).find('.btn-success').size()>0) {
		     $(this).find('.btn').toggleClass('btn-success');
		}
		if ($(this).find('.btn-info').size()>0) {
		   $(this).find('.btn').toggleClass('btn-info');
		}

    	        $(this).find('.btn').toggleClass('btn-default');
	});

</script>
   ]]

dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
