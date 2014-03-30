--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "graph_utils"

-- Toggle debug
local enable_second_debug = 0

ifnames = interface.getIfNames()
for _,ifname in pairs(ifnames) do
   a = string.ends(ifname, ".pcap")
   if(not(a)) then 
      interface.find(ifname)
      ifstats = interface.getStats()
      dirs = ntop.getDirs()
      basedir = fixPath(dirs.workingdir .. "/" .. purifyInterfaceName(ifname) .. "/rrd")

      -- io.write(basedir.."\n")
      if(not(ntop.exists(basedir))) then
	 if(enable_second_debug == 1) then io.write('Creating base directory ', basedir, '\n') end
	 ntop.mkdir(basedir)
      end

      -- Traffic stats
      name = fixPath(basedir .. "/" .. "bytes.rrd")
      create_rrd(name,"bytes")
      ntop.rrd_update(name, "N:".. ifstats.stats_bytes)
      if(enable_second_debug == 1) then io.write('Updating RRD '.. name .. " " .. ifstats.stats_bytes ..'\n') end

      name = fixPath(basedir .. "/" .. "packets.rrd")
      create_rrd(name,"packets")
      ntop.rrd_update(name, "N:".. ifstats.stats_packets)
      if(enable_second_debug == 1) then io.write('Updating RRD '.. name ..'\n') end
   end
end -- for _,ifname in pairs(ifnames) do