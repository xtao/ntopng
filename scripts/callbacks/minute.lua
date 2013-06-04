--
-- (C) 2013 - ntop.org
--

package.path = "./scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"
require "top_talkers"

when = os.time()

-- Dump topTalkers every minute

talkers = getTopTalkers("any")
basedir = ntop.getDataDir() .. "/talkers/" .. os.date("%Y/%m/%d/%H", when)
if(not(ntop.exists(basedir))) then   
  ntop.mkdir(basedir)
end
filename = basedir .. os.date("/%M.json", when)

-- io.write(filename)

f = io.open(filename, "w")
if(f) then
  f:write(talkers)
  f:close()
end






-- Run RRD update every 5 minutes
-- Use 30 just to avoid rounding issues
diff = when % 300

-- io.write('Diff: '..diff..'\n')

-- Toggle debug
local enable_minute_debug = 0

if(enable_minute_debug == 0) then
   if(diff > 30) then
      return
   end
end

interface.find("any")
hosts_stats = interface.getHostsInfo()
for key, value in pairs(hosts_stats) do
   if(enable_minute_debug == 1) then print ("[" .. key .. "][" .. (hosts_stats[key]["bytes.sent"]+hosts_stats[key]["bytes.rcvd"]) .. "]\n") end

   if(hosts_stats[key]["localhost"] == true) then
      basedir = ntop.getDataDir() .. "/rrd/" .. key

      if(not(ntop.exists(basedir))) then
	 if(enable_minute_debug == 1) then io.write('Creating base directory ', basedir, '\n') end
	 ntop.mkdir(basedir)
      end

      -- Traffic stats
      name = basedir .. "/bytes.rrd"
      if(not(ntop.exists(name))) then
	 if(enable_minute_debug == 1) then io.write('Creating RRD ', name, '\n') end
	 rrd.create(
	    name,
	    '--start', 'now',
	    '--step', '300',
	    'DS:sent:DERIVE:600:U:U',
	    'DS:rcvd:DERIVE:600:U:U',
	    'RRA:AVERAGE:0.5:1:50400',  -- raw: 7 days = 7 * 24 = 168 * 300 sec = 50400
	    'RRA:AVERAGE:0.5:12:2400',  -- 1h resolution (12 points)   2400 hours = 100 days
	    'RRA:AVERAGE:0.5:288:365',  -- 1d resolution (288 points)  365 days
	    'RRA:HWPREDICT:1440:0.1:0.0035:20')
	 
      end
      rrd.update(name, "N:"..hosts_stats[key]["bytes.sent"] .. ":" .. hosts_stats[key]["bytes.rcvd"])
      if(enable_minute_debug == 1) then io.write('Updating RRD '..name..'\n') end

      -- L4 Protocols
      host = interface.getHostInfo(key)
      if((host ~= nil) and host.localhost) then	 
	 for id, _ in ipairs(l4_keys) do
	    k = l4_keys[id][2]

	    if(enable_minute_debug == 1) then print("\t"..k.."\n") end

	    name = basedir .. "/".. k .. ".rrd"
	    if(not(ntop.exists(name))) then
	       if(enable_minute_debug == 1) then io.write('Creating RRD ', name, '\n') end
	       rrd.create(
		  name,
		  '--start', 'now',
		  '--step', '300',
		  'DS:sent:DERIVE:600:U:U',
		  'DS:rcvd:DERIVE:600:U:U',
		  'RRA:AVERAGE:0.5:1:50400',  -- raw: 7 days = 7 * 24 = 168 * 300 sec = 50400
		  'RRA:AVERAGE:0.5:12:2400',  -- 1h resolution (12 points)   2400 hours = 100 days
		  'RRA:AVERAGE:0.5:288:365',  -- 1d resolution (288 points)  365 days
		  'RRA:HWPREDICT:1440:0.1:0.0035:20')
	    end

	    rrd.update(name, "N:".. host[k..".bytes.sent"] .. ":" .. host[k..".bytes.rcvd"])
	    if(enable_minute_debug == 1) then io.write('Updating RRD '..name..'\n') end
	 end
      end

      -- nDPI Protocols
      host = interface.getHostInfo(key)
      if((host ~= nil) and host.localhost) then
	 for k in pairs(host["ndpi"]) do
	    name = basedir .. "/".. k .. ".rrd"
	    if(not(ntop.exists(name))) then
	       if(enable_minute_debug == 1) then io.write('Creating RRD ', name, '\n') end
	       rrd.create(
		  name,
		  '--start', 'now',
		  '--step', '300',
		  'DS:sent:DERIVE:600:U:U',
		  'DS:rcvd:DERIVE:600:U:U',
		  'RRA:AVERAGE:0.5:1:50400',  -- raw: 7 days = 7 * 24 = 168 * 300 sec = 50400
		  'RRA:AVERAGE:0.5:12:2400',  -- 1h resolution (12 points)   2400 hours = 100 days
		  'RRA:AVERAGE:0.5:288:365',  -- 1d resolution (288 points)  365 days
		  'RRA:HWPREDICT:1440:0.1:0.0035:20')
	    end

	    rrd.update(name, "N:".. host["ndpi"][k]["bytes.sent"] .. ":" .. host["ndpi"][k]["bytes.rcvd"])
	    if(enable_minute_debug == 1) then io.write('Updating RRD '..name..'\n') end
	 end
      end

   end -- if
end -- for

