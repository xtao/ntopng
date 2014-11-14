--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "graph_utils"
require "influx_utils"

-- Toggle debug
local enable_second_debug = 0

if(use_influx) then
   cache_key = "second.lua.cache"
   load_last_influx(cache_key)
   when = os.time().."000"
   header = '[\n  {\n "name" : "interfaces",\n "columns" : ["time", "name", "bytes", "packets"],\n "points" : [\n'
   num = 0
end

ifnames = interface.getIfNames()
for _,ifname in pairs(ifnames) do
   a = string.ends(ifname, ".pcap")
   if(not(a)) then
      interface.find(ifname)
      ifstats = interface.getStats()
      dirs = ntop.getDirs()
      basedir = fixPath(dirs.workingdir .. "/" .. ifstats.id .. "/rrd")

      -- io.write(basedir.."\n")
      if(not(ntop.exists(basedir))) then
	 if(enable_second_debug == 1) then io.write('Creating base directory ', basedir, '\n') end
	 ntop.mkdir(basedir)
      end

      -- Traffic stats
      name = fixPath(basedir .. "/" .. "bytes.rrd")
      create_rrd(name, "bytes")
      ntop.rrd_update(name, "N:".. ifstats.stats_bytes)
      if(enable_second_debug == 1) then io.write('Updating RRD ['.. ifname..'] '.. name .. " " .. ifstats.stats_bytes ..'\n') end

      name = fixPath(basedir .. "/" .. "packets.rrd")
      create_rrd(name, "packets")
      ntop.rrd_update(name, "N:".. ifstats.stats_packets)
      if(enable_second_debug == 1) then io.write('Updating RRD ['.. ifname..'] '.. name ..'\n') end

      if(use_influx and (ifstats.stats_bytes > 0)) then
	 b = diff_value_influx(ifstats.name, "bytes", ifstats.stats_bytes)
	 p = diff_value_influx(ifstats.name, "packets", ifstats.stats_packets)
	 if(b > 0) then
	    if(num > 0) then header = header .. ',\n' end
	    header = header .. '['.. when .. ', "' .. ifstats.name .. '",' .. b .. ',' .. p .. ']'
	    num = num + 1
	 end
      end
   end
end -- for _,ifname in pairs(ifnames) do

if(use_influx) then
   header = header .. "\n]\n }\n]\n"
   --io.write(header)
   ntop.postHTTPJsonData(influx_user, influx_pwd, influx_url, header)
   save_curr_influx(cache_key)
end