--
-- (C) 2013 - ntop.org
--

-- Run RRD update every 5 minutes
-- Use 30 just to avoid rounding issues
diff = os.time() % 300

-- io.write('Diff: '..diff..'\n')

if(diff > 30) then
   return
end

-- Toggle debug
debug = 0

interface.find("any")
hosts_stats = interface.getHostsInfo()
for key, value in pairs(hosts_stats) do
   -- print ("[" .. key .. "][" .. (hosts_stats[key]["bytes.sent"]+hosts_stats[key]["bytes.rcvd"]) .. "]\n")
   
   basedir = ntop.getDataDir() .. "/rrd/" .. key

   if(not(ntop.exists(basedir))) then
      if(debug) then io.write('Creating base directory ', basedir, '\n') end
      ntop.mkdir(basedir)
   end

   -- Traffic stats
   name = basedir .. "/bytes.rrd"
   if(not(ntop.exists(name))) then
      if(debug) then io.write('Creating RRD ', name, '\n') end
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
   if(debug) then io.write('Updating RRD '..name..'\n') end

   -- nDPI Protocols
   host = interface.getHostInfo(key)
   if((host ~= nil) and host.localhost) then
      for k in pairs(host["ndpi"]) do
	 name = basedir .. "/".. k .. ".rrd"
	 if(not(ntop.exists(name))) then
	    if(debug) then io.write('Creating RRD ', name, '\n') end
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
	 rrd.update(name, "N:".. host["ndpi"][k]["sent"] .. ":" .. host["ndpi"][k]["rcvd"])
	 if(debug) then io.write('Updating RRD '..name..'\n') end
      end
   end
end

