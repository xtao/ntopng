--
-- (C) 2013 - ntop.org
--


dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "top_talkers"

when = os.time()

local verbose = ntop.verboseTrace()

ifnames = interface.getIfNames()
num_ifaces = 0
for _,ifname in pairs(ifnames) do

if(verbose) then print("[minute.lua] Processing interface " .. ifname) end
-- Dump topTalkers every minute

talkers = getTopTalkers(ifname)
basedir = dirs.workingdir .. "/" .. ifname .. "/top_talkers/" .. os.date("%Y/%m/%d/%H", when)
if(not(ntop.exists(basedir))) then   
  ntop.mkdir(basedir)
end
filename = basedir .. os.date("/%M.json", when)

if(verbose) then print("[minute.lua] Creating "..filename) end

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
if(verbose == 0) then
   if(diff > 30) then
      return
   end
end

interface.find(ifname)
hosts_stats = interface.getHostsInfo()
for key, value in pairs(hosts_stats) do
   if(verbose == 1) then print ("[" .. key .. "][" .. (hosts_stats[key]["bytes.sent"]+hosts_stats[key]["bytes.rcvd"]) .. "]\n") end

   if(hosts_stats[key]["localhost"] == true) then
      basedir = dirs.workingdir .. "/" .. ifname .. "/rrd/" .. key

      if(not(ntop.exists(basedir))) then
	 if(verbose == 1) then io.write('Creating base directory ', basedir, '\n') end
	 ntop.mkdir(basedir)
      end

      -- Traffic stats
      name = basedir .. "/bytes.rrd"
      if(not(ntop.exists(name))) then
	 if(verbose == 1) then io.write('Creating RRD ', name, '\n') end
	 ntop.rrd_create(
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
      ntop.rrd_update(name, "N:"..hosts_stats[key]["bytes.sent"] .. ":" .. hosts_stats[key]["bytes.rcvd"])
      if(verbose == 1) then io.write('Updating RRD '..name..'\n') end

      -- L4 Protocols
      host = interface.getHostInfo(key)
      if((host ~= nil) and host.localhost) then	 
	 for id, _ in ipairs(l4_keys) do
	    k = l4_keys[id][2]

	    if(verbose == 1) then print("\t"..k.."\n") end

	    name = basedir .. "/".. k .. ".rrd"
	    if(not(ntop.exists(name))) then
	       if(verbose == 1) then io.write('Creating RRD ', name, '\n') end
	       ntop.rrd_create(
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

	    ntop.rrd_update(name, "N:".. host[k..".bytes.sent"] .. ":" .. host[k..".bytes.rcvd"])
	    if(verbose == 1) then io.write('Updating RRD '..name..'\n') end
	 end
      end

      -- nDPI Protocols
      host = interface.getHostInfo(key)
      if((host ~= nil) and host.localhost) then
	 for k in pairs(host["ndpi"]) do
	    name = basedir .. "/".. k .. ".rrd"
	    if(not(ntop.exists(name))) then
	       if(verbose == 1) then io.write('Creating RRD ', name, '\n') end
	       ntop.rrd_create(
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

	    ntop.rrd_update(name, "N:".. host["ndpi"][k]["bytes.sent"] .. ":" .. host["ndpi"][k]["bytes.rcvd"])
	    if(verbose == 1) then io.write('Updating RRD '..name..'\n') end
	 end
      end

   end -- if
end -- for

end -- for ifname,_ in pairs(ifnames) do