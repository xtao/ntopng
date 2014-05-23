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

     <style type="text/css">
     #map-canvas { width: 640px; height: 480px; }
   </style>

<hr>
<h2>Export Data</H2>
<p>&nbsp;<p>

<form class="form-horizontal" action="/lua/do_export_data.lua" method="GET">


<div class="control-group">
    <label class="control-label" for="hostIP">Host:</label>
    <div class="controls">
      <input type="text" id="hostIP" name="hostIP" placeholder="IP or MAC Address">

     <label><small>NOTE: If the field is empty all hosts will be exported</small></label>
    </div>
  </div>

<div class="control-group">
    <label class="control-label" for="hostVlan">Vlan:</label>
    <div class="controls">
      <input type="text" id="hostVlan" name="hostVlan" placeholder="Vlan">

     <label><small>NOTE: If the field is empty vlan is set to 0.</small></label>
    </div>
  </div>

<div class="control-group">
<div class="controls">
<button type="submit" class="btn btn-primary">Export JSON Data</button> <button class="btn" type="reset">Reset Form</button>
</div>
</div>

<script type='text/javascript'>
  function auto_ip_mac () {
   $('#hostIP').typeahead({
       source: function (query, process) {
               return $.get('/lua/find_host.lua', { query: query }, function (data) {
                     return process(data.results);
      });
      }
    });
  }

  $(document).ready(function(){
    auto_ip_mac();
  });
</script>


</form>

]]


dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")