--
-- (C) 2013 - ntop.org
--

-- Toggle debug
local enable_second_debug = 0

function create_rrd(name, ds)
   if(not(ntop.exists(name))) then
      if(enable_second_debug == 1) then io.write('Creating RRD ', name, '\n') end
      ntop.rrd_create(
	 name,
	 '--start', 'now',
	 '--step', '1',
	 'DS:' .. ds .. ':DERIVE:5:U:U',
	 'RRA:AVERAGE:0.5:1:86400',    -- raw: 1 day = 86400
	 'RRA:AVERAGE:0.5:3600:2400',  -- 1h resolution (3600 points)   2400 hours = 100 days
	 'RRA:AVERAGE:0.5:86400:365',    -- 1d resolution (86400 points)  365 days
	 'RRA:HWPREDICT:1440:0.1:0.0035:20')
   end
end

ifname = "any"
interface.find(ifname)
ifstats = interface.getStats()
dirs = ntop.getDirs()
basedir = dirs.workingdir .. "/rrd/interface."..ifname

if(not(ntop.exists(basedir))) then
   if(enable_second_debug == 1) then io.write('Creating base directory ', basedir, '\n') end
   ntop.mkdir(basedir)
end

-- Traffic stats
name =  basedir .. "/" .. "bytes.rrd"
create_rrd(name,"bytes")
ntop.rrd_update(name, "N:".. ifstats.stats_bytes)
if(enable_second_debug == 1) then io.write('Updating RRD '.. name .. " " .. ifstats.stats_bytes ..'\n') end

name =  basedir .. "/" .. "packets.rrd"
create_rrd(name,"packets")
ntop.rrd_update(name, "N:".. ifstats.stats_packets)
if(enable_second_debug == 1) then io.write('Updating RRD '.. name ..'\n') end
