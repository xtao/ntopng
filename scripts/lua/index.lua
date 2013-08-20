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

active_page = "home"
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

page = _GET["page"]
if(page == nil) then page = "TopFlowTalkers" end

if(ifstats.stats_packets > 0) then
-- Print tabbed header

   print('<div class="navbar">\n\t<div class="navbar-inner">\n\t<ul class="nav">\n')

   print('<li><a href="#">Top: </a></li>\n')

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
print("<div class=\"alert alert-warning\">No packet has been received yet on interface " .. ifname .. ".<p>Please wait and then reload this page in a few seconds.</div>")
end

  dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")