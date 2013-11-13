--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')

interface.find(ifname)
ifstats = interface.getStats()

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")

-- NOTE: in the home page, footer.lua checks the ntopng version
-- so in case we change it, footer.lua must also be updated
active_page = "home"
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

page = _GET["page"]
if(page == nil) then page = "TopFlowTalkers" end

if(ifstats.stats_packets > 0) then
-- Print tabbed header

   print('<div class="navbar">\n\t<div class="navbar-inner">\n\t<ul class="nav">\n')

   print('<li><a href="#">Dashboard: </a></li>\n')

   if(page == "TopFlowTalkers") then active=' class="active"' else active = "" end
   print('<li'..active..'><a href="/?page=TopFlowTalkers">Talkers</a></li>\n')

   if((page == "TopHosts")) then active=' class="active"' else active = "" end
   print('<li'..active..'><a href="/?page=TopHosts">Hosts</a></li>\n')


   if((page == "TopApplications")) then active=' class="active"' else active = "" end
   print('<li'..active..'><a href="/?page=TopApplications">Applications</a></li>\n')

   if((page == "TopASNs")) then active=' class="active"' else active = "" end
   print('<li'..active..'><a href="/?page=TopASNs">ASNs</a></li>\n')

   if((page == "TopFlowSenders")) then active=' class="active"' else active = "" end
   print('<li'..active..'><a href="/?page=TopFlowSenders">Senders</a></li>\n')

   print('</ul>\n\t</div>\n\t</div>\n')

   if(page == "TopFlowTalkers") then
      print('<div class="jumbotron">\n<h4>Top Flow Talkers</h4>\n')      
      dofile(dirs.installdir .. "/scripts/lua/inc/sankey.lua")
      print('\n</div>\n')
      else 
   ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/index_" .. page .. ".inc")
   end


  --ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/index_top.inc")
  -- ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/index_bottom.inc")
else
print("<div class=\"alert alert-warning\">No packet has been received yet on interface " .. ifname .. ".<p>Please wait <span id='countdown'></span> seconds until this page reloads.</div> <script type=\"text/JavaScript\">(function countdown(remaining) { if(remaining <= 0) location.reload(true); document.getElementById('countdown').innerHTML = remaining;  setTimeout(function(){ countdown(remaining - 1); }, 1000);})(10);</script>")

end

info = ntop.getInfo()

if(page == "TopFlowTalkers") then
   rsp = ntop.httpGet("www.ntop.org", "/ntopng.version")
   
   version_elems = split(info["version"], " ");
   
   stable_version = version2int(rsp)
   this_version   = version2int(version_elems[1])
   
   if(stable_version > this_version) then
      print("<p><div class=\"alert alert-ok\"><i class=\"fa fa-icon-cloud-download fa-lg\"></i> A new ntopng version (v." .. rsp .. ") is available for <A HREF=http://www.ntop.org>download</A>: please upgrade.</div></p>")
   end
end

dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")