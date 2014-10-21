--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path
-- io.write ("Session:".._SESSION["session"].."\n")
require "lua_utils"

sendHTTPHeader('text/html; charset=iso-8859-1')

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")

dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")
-- Body


function pieChart(div_name, url) 
print [[

<div class='chart'>
<div id=']] print(div_name) print [['></div>
</div>

<script>
var ]] print(div_name) print [[ = c3.generate({
    bindto: '#]] print(div_name) print [[',
    transition: { duration: 0 },
    color: { pattern: ['#1f77b4', '#aec7e8', '#ff7f0e', '#ffbb78', '#2ca02c', '#98df8a' ] },

    data: {
        columns: [ ],
        type : 'donut',
    }
});

function update_]] print(div_name) print [[() {
    $.ajax({
      type: 'GET',
        url: ']] print(url) print [[',
	  data: {  },
          success: function(content) {
	      try {
   	         data = jQuery.parseJSON(content);
	         ]] print(div_name) print [[.load({ columns: data })
	        } catch(e) {
		     console.log(e);
  	        }
          }
      });
}

$(document).ready(function () { update_]] print(div_name) print [[(); });
setInterval(function() { update_]] print(div_name) print [[(); }, 3000);

</script>



]]

end




print [[
<div class="container-fluid">

<div class="row">
  <div class="col-md-4">
     <div class="panel panel-default">
        <div class="panel-heading">Empty</div>
        <div class="panel-body">]] pieChart("top_test", "/lua/iface_hosts_list.lua?ajax_format=c3")   print [[</div>
     </div>
  </div>

  <div class="col-md-4">
     <div class="panel panel-default">
        <div class="panel-heading">Empty</div>
        <div class="panel-body">Body</div>
     </div>
  </div>

  <div class="col-md-4">
     <div class="panel panel-default">
        <div class="panel-heading">Interfaces</div>
        <div class="panel-body">Body</div>
     </div>
  </div>
</div>
<div class="row">
  <div class="col-md-4">
     <div class="panel panel-default">
        <div class="panel-heading">Top Senders</div>
        <div class="panel-body">]] pieChart("top_senders", "/lua/iface_hosts_list.lua?ajax_format=c3")   print [[</div>
     </div>
  </div>

  <div class="col-md-4">
     <div class="panel panel-default">
        <div class="panel-heading">Top Receivers</div>
        <div class="panel-body">Body</div>
     </div>
  </div>
  <div class="col-md-4">
     <div class="panel panel-default">
        <div class="panel-heading">Empty</div>
        <div class="panel-body">Body</div>
     </div>
  </div>

</div>


</div>
]]


-- Footer
dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")

