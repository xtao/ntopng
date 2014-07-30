--
-- (C) 2013 - ntop.org
--


dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "top_talkers"
require "alert_utils"
require "graph_utils"

when = os.time()

local verbose = ntop.verboseTrace()

-- Scan "minute" alerts
scanAlerts("min")

ifnames = interface.getIfNames()
num_ifaces = 0
verbose = false

if((_GET ~= nil) and (_GET["verbose"] ~= nil)) then
   verbose = true
end

if(verbose) then
   sendHTTPHeader('text/plain')
end

host_rrd_creation = ntop.getCache("ntopng.prefs.host_rrd_creation")
host_ndpi_rrd_creation = ntop.getCache("ntopng.prefs.host_ndpi_rrd_creation")

-- id = 0
for _,_ifname in pairs(ifnames) do
   interface.find(_ifname)
   ifstats = interface.getStats()

   if(verbose) then print("\n["..__FILE__()..":"..__LINE__().."]===============================\n["..__FILE__()..":"..__LINE__().."] Processing interface " .. _ifname .. " ["..ifstats.id.."]") end
   -- Dump topTalkers every minute

   talkers = getTopTalkers(ifstats.id, _ifname)
   basedir = fixPath(dirs.workingdir .. "/" .. ifstats.id .. "/top_talkers/" .. os.date("%Y/%m/%d/%H", when))
   filename = fixPath(basedir .. os.date("/%M.json", when))

   if(not(ntop.exists(basedir))) then
      if(verbose) then print('\n["..__FILE__()..":"..__LINE__().."] Creating base directory ', basedir, '\n') end
      ntop.mkdir(basedir)
   end

   if(verbose) then print("\n["..__FILE__()..":"..__LINE__().."] Creating "..filename.."\n") end

   f = io.open(filename, "w")
   if(f) then
      f:write(talkers)
      f:close()
   end

   -- Run RRD update every 5 minutes
   -- Use 30 just to avoid rounding issues
   diff = when % 300

   -- print('\n["..__FILE__()..":"..__LINE__().."] Diff: '..diff..'\n')

   if(verbose or (diff < 60)) then
      -- Scan "5 minute" alerts
      scanAlerts("5mins")

      basedir = fixPath(dirs.workingdir .. "/" .. ifstats.id .. "/rrd")	
      for k in pairs(ifstats["ndpi"]) do
	 v = ifstats["ndpi"][k]["bytes.sent"]+ifstats["ndpi"][k]["bytes.rcvd"]
	 if(verbose) then print("["..__FILE__()..":"..__LINE__().."] ".._ifname..": "..k.."="..v.."\n") end

         name = fixPath(basedir .. "/"..k..".rrd")
         createSingleRRDcounter(name, verbose)
         ntop.rrd_update(name, "N:".. v)
      end

      -- Save hosts stats
      if(host_rrd_creation ~= "0") then      
      hosts_stats = interface.getHostsInfo()
      for key, value in pairs(hosts_stats) do
	 host = interface.getHostInfo(key)

	 if(host == nil) then
	    if(verbose) then print("\n["..__FILE__()..":"..__LINE__().."] NULL host "..key.." !!!!\n") end
	 else
	    if(verbose) then
	       print ("["..__FILE__()..":"..__LINE__().."] [" .. key .. "][local: ")
	       print(host["localhost"])
	       print("]" .. (hosts_stats[key]["bytes.sent"]+hosts_stats[key]["bytes.rcvd"]) .. "]\n")
	    end

	    if(host.localhost) then
	       basedir = fixPath(dirs.workingdir .. "/" .. ifstats.id .. "/rrd/" .. key)

	       if(not(ntop.exists(basedir))) then
		  if(verbose) then print("\n["..__FILE__()..":"..__LINE__().."] Creating base directory ", basedir, '\n') end
		  ntop.mkdir(basedir)
	       end

	       -- Traffic stats
	       name = fixPath(basedir .. "/bytes.rrd")
	       createRRDcounter(name, verbose)
	       ntop.rrd_update(name, "N:"..hosts_stats[key]["bytes.sent"] .. ":" .. hosts_stats[key]["bytes.rcvd"])
	       if(verbose) then print("\n["..__FILE__()..":"..__LINE__().."] Updating RRD [".. ifstats.name .."] "..name..'\n') end

	       -- L4 Protocols
	       for id, _ in ipairs(l4_keys) do
		  k = l4_keys[id][2]
		  if((host[k..".bytes.sent"] ~= nil) and (host[k..".bytes.rcvd"] ~= nil)) then
		     if(verbose) then print("["..__FILE__()..":"..__LINE__().."]\t"..k.."\n") end

		     name = fixPath(basedir .. "/".. k .. ".rrd")
		     createRRDcounter(name, verbose)
		     -- io.write(name.."="..host[k..".bytes.sent"].."|".. host[k..".bytes.rcvd"] .. "\n")
		     ntop.rrd_update(name, "N:".. host[k..".bytes.sent"] .. ":" .. host[k..".bytes.rcvd"])
		     if(verbose) then print("\n["..__FILE__()..":"..__LINE__().."] Updating RRD [".. ifstats.name .."] "..name..'\n') end
		  else
		     -- L2 host
		     --io.write("Discarding "..k.."@"..key.."\n")
		  end
	       end

       	       if(host_ndpi_rrd_creation ~= "0") then
	         -- nDPI Protocols
	         for k in pairs(host["ndpi"]) do
		  name = fixPath(basedir .. "/".. k .. ".rrd")
		  createRRDcounter(name, verbose)
		  ntop.rrd_update(name, "N:".. host["ndpi"][k]["bytes.sent"] .. ":" .. host["ndpi"][k]["bytes.rcvd"])
		  if(verbose) then print("\n["..__FILE__()..":"..__LINE__().."] Updating RRD [".. ifstats.name .."] "..name..'\n') end
	         end

	         if(host["epp"]) then dumpSingleTreeCounters(basedir, "epp", host, verbose) end
	         if(host["dns"]) then dumpSingleTreeCounters(basedir, "dns", host, verbose) end
               end
	    else
	       if(verbose) then print("["..__FILE__()..":"..__LINE__().."] Skipping non local host "..key.."\n") end
	    end
	 end -- if
      end -- for
     end -- if rrd
   end -- if(diff
end -- for ifname,_ in pairs(ifnames) do
