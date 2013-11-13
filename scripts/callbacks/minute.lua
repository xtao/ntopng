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
--verbose = 1
for _,_ifname in pairs(ifnames) do
   ifname = purifyInterfaceName(_ifname)
   if(verbose) then print("[minute.lua] Processing interface " .. ifname) end
   -- Dump topTalkers every minute

   talkers = getTopTalkers(_ifname)
   basedir = fixPath(dirs.workingdir .. "/" .. ifname .. "/top_talkers/" .. os.date("%Y/%m/%d/%H", when))
   if(not(ntop.exists(basedir))) then   
      ntop.mkdir(basedir)
   end
   filename = fixPath(basedir .. os.date("/%M.json", when))

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
   
   if(diff < 60) then
      
      interface.find(_ifname)
      hosts_stats = interface.getHostsInfo()
      for key, value in pairs(hosts_stats) do
	 if(verbose == 1) then print ("[" .. key .. "][" .. (hosts_stats[key]["bytes.sent"]+hosts_stats[key]["bytes.rcvd"]) .. "]\n") end
	 
	 if(hosts_stats[key]["localhost"] == true) then
	    basedir = fixPath(dirs.workingdir .. "/" .. ifname .. "/rrd/" .. key)
	    
	    if(not(ntop.exists(basedir))) then
	       if(verbose == 1) then io.write('Creating base directory ', basedir, '\n') end
	       ntop.mkdir(basedir)
	    end
	    
	    -- Traffic stats
	    name = fixPath(basedir .. "/bytes.rrd")
	    if(not(ntop.exists(name))) then
	       if(verbose == 1) then io.write('Creating RRD ', name, '\n') end
	       ntop.rrd_create(
		  name,
		  '--start', 'now',
		  '--step', '300',
		  'DS:sent:DERIVE:600:U:U',
		  'DS:rcvd:DERIVE:600:U:U',
		  'RRA:AVERAGE:0.5:1:7200',  -- raw: 1 day = 1 * 24 = 24 * 300 sec = 7200
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
		  
		  if((host[k..".bytes.sent"] ~= nil) and (host[k..".bytes.rcvd"] ~= nil)) then
		     if(verbose == 1) then print("\t"..k.."\n") end
		     
		     name = fixPath(basedir .. "/".. k .. ".rrd")
		     if(not(ntop.exists(name))) then
			if(verbose == 1) then io.write('Creating RRD ', name, '\n') end
			ntop.rrd_create(
			   name,
			   '--start', 'now',
			   '--step', '300',
			   'DS:sent:DERIVE:600:U:U',
			   'DS:rcvd:DERIVE:600:U:U',
			   'RRA:AVERAGE:0.5:1:7200',  -- raw: 1 day = 1 * 24 = 24 * 300 sec = 7200
			   'RRA:AVERAGE:0.5:12:2400',  -- 1h resolution (12 points)   2400 hours = 100 days
			   'RRA:AVERAGE:0.5:288:365',  -- 1d resolution (288 points)  365 days
			   'RRA:HWPREDICT:1440:0.1:0.0035:20')
		     end
		     
		     -- io.write(name.."="..host[k..".bytes.sent"].."|".. host[k..".bytes.rcvd"] .. "\n")
		     ntop.rrd_update(name, "N:".. host[k..".bytes.sent"] .. ":" .. host[k..".bytes.rcvd"])
		     if(verbose == 1) then io.write('Updating RRD '..name..'\n') end
		  else
		     -- L2 host
		     --io.write("Discarding "..k.."@"..key.."\n")
		  end
	       end
	    end
	    
	    -- nDPI Protocols
	    host = interface.getHostInfo(key)
	    if((host ~= nil) and host.localhost) then
	       for k in pairs(host["ndpi"]) do
		  name = fixPath(basedir .. "/".. k .. ".rrd")
		  if(not(ntop.exists(name))) then
		     if(verbose == 1) then io.write('Creating RRD ', name, '\n') end
		     ntop.rrd_create(
			name,
			'--start', 'now',
			'--step', '300',
			'DS:sent:DERIVE:600:U:U',
			'DS:rcvd:DERIVE:600:U:U',
			'RRA:AVERAGE:0.5:1:7200',  -- raw: 1 day = 1 * 24 = 24 * 300 sec = 7200
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
   end -- if(diff

end -- for ifname,_ in pairs(ifnames) do
