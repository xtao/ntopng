--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')


ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")
active_page = "hosts"
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

interface.find(ifname)
hosts_stats = interface.getHostsInfo()
num = 0
for key, value in pairs(hosts_stats) do
    num = num + 1
end


if(num > 0) then
print [[

     <style type="text/css">
     #map-canvas { width: 640px; height: 480px; }
   </style>

<hr>
<h2>Hosts GeoMap</H2>

    <script src="https://maps.googleapis.com/maps/api/js?v=3.exp&sensor=false"></script>
<div class="container-fluid">
  <div class="row-fluid">
    <div class="span8">
      <div id="map-canvas"></div>
<p>&nbsp;<p><small><b>NOTE</b></small>
<ol>
<li> <small>In order to visualize maps you must:</small>
<ol>
<li> <small>Have a working Internet connection</small>
<li> <small>Have compiled ntopng with geolocation and started with it</small>
</ol>
<li> <small>HTML <A HREF=http://diveintohtml5.info/geolocation.html>browser geolocation</A> is used to place on map hosts with private IP addresses or unknown locations.</small>
</ul>
</small>
</div>
</div>
</div>

    <script type="text/javascript" src="/js/googleMapJson.js" ></script>



</body>
</html>
]]
else 
print("<div class=\"alert alert-error\"><img src=/img/warning.png> No results found</div>")
end

dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")