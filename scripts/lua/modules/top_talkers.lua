--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "persistence"

function getTopTalkers(ifname, mode, epoch)
   if(ifname == nil) then ifname = "any" end

   if(epoch ~= nil) then
      rsp = getHistoricalTopTalkers(ifname, mode, epoch)
   else
      rsp = getActualTopTalkers(ifname, mode)
   end

   return(rsp)
end


-- #################################################

function getHistoricalTopTalkers(ifname, mode, epoch)
   epoch = epoch - (epoch % 60)
   dirs = ntop.getDirs()
   filename = dirs.workingdir .. "/top_talkers/" .. ifname .. os.date("/%Y/%m/%d/%H", epoch) .. os.date("/%M.json", epoch)

   if(ntop.exists(filename)) then
      f = io.open(filename, "r")
      if(f) then
	 rsp = ""
	 while(true) do
	    line = f:read()

	    if(line == nil) then break end
	    rsp = rsp .. line.."\n"
	 end
	 f:close()
      end

      return(rsp)
   else
      return("[ ]\n")
   end
end
-- #################################################

function getActualTopTalkers(ifname, mode, epoch)
   max_num_entries = 10
   rsp = ""

   interface.find(ifname)
   hosts_stats = interface.getFlowsInfo()

   sent = {}
   _sent = {}
   rcvd = {}
   _rcvd = {}

   for _key, value in pairs(hosts_stats) do
      key = hosts_stats[_key]["src.ip"]
      if(key ~= nil) then
	 old = _sent[key]
	 if(old == nil) then old = 0 end
	 _sent[key] = old + hosts_stats[_key]["cli2srv.bytes"]
      end

      key = hosts_stats[_key]["dst.ip"]
      if(key ~= nil) then
	 old = _rcvd[key]
	 if(old == nil) then old = 0 end
	 _rcvd[key] = old + hosts_stats[_key]["cli2srv.bytes"]
      end

      -- ###########################

      key = hosts_stats[_key]["dst.ip"]
      if(key ~= nil) then
	 old = _sent[key]	
	 if(old == nil) then old = 0 end
	 _sent[key] = old + hosts_stats[_key]["srv2cli.bytes"]

	 key = hosts_stats[_key]["src.ip"]
	 if(key ~= nil) then
	    old = _rcvd[key]
	    if(old == nil) then old = 0 end
	    _rcvd[key] = old + hosts_stats[_key]["srv2cli.bytes"]
	 end
      end
   end

   if(mode == nil) then
      rsp = rsp .. "{\n"
      rsp = rsp .. '\t"senders": ['
   else
      rsp = rsp .. "[\n"
   end

   -- io.write("Hello\n")
   if((mode == nil) or (mode == "senders")) then
      talkers_dir = dirs.workingdir .. "/top_talkers/" .. ifname
      if(not(ntop.exists(talkers_dir))) then   
	 ntop.mkdir(talkers_dir)
      end

      -- Read the lastdump      
      lastdump = talkers_dir .. "/.sent_lastdump"
      if(ntop.exists(lastdump)) then
	 last = persistence.load(lastdump)
      else
	 last = {}
      end

      persistence.store(lastdump, _sent);

      for key, value in pairs(_sent) do
	 if(last[key] ~= nil) then
	    v = _sent[key]-last[key]

	    if(v < 0) then v = 0 end
	    _sent[key] = v
	 end
      end

      for key, value in pairs(_sent) do
	 sent[value] = key
      end

      -- Compute traffic
      total = 0
      num = 0
      for _key, _value in pairsByKeys(sent, rev) do
	 total = total + _key
      end

      -- 10 %
      threshold = total / 10

      num = 0
      for _key, _value in pairsByKeys(sent, rev) do
	 key   = sent[_key]
	 value = _key

	 if((value == 0) or ((value < threshold) and (num > 2))) then break end
	 if(num > 0) then rsp = rsp .. " }," end
	 rsp = rsp .. '\n\t\t { "label": "'..key..'", "value": '..value
	 num = num + 1

	 if(num == max_num_entries) then
	    break
	 end
      end


   end

   if(mode == nil) then
      if(num > 0) then rsp = rsp .. " }" end
      rsp = rsp .. "\n\t],\n"
      rsp = rsp .. '\t"receivers": ['
   end

   if((mode == nil) or (mode == "receivers")) then
      -- Read the lastdump
      lastdump = dirs.workingdir .. "/top_talkers/" .. ifname .. "/.rcvd_lastdump"
      if(ntop.exists(lastdump)) then
	 last = persistence.load(lastdump)
      else
	 last = {}
      end

      persistence.store(lastdump, _rcvd);

      for key, value in pairs(_rcvd) do
	 -- io.write(key.."\n")

	 if(last[key] ~= nil) then
	    v = _rcvd[key]-last[key]

	    if(v < 0) then v = 0 end
	    _rcvd[key] = v
	 end
      end

      for key, value in pairs(_rcvd) do
	 rcvd[value] = key
      end

      -- Compute traffic
      total = 0
      num = 0
      for _key, _value in pairsByKeys(rcvd, rev) do
	 total = total + _key
      end

      -- 10 %
      threshold = total / 10

      num = 0
      for _key, _value in pairsByKeys(rcvd, rev) do
	 key   = rcvd[_key]
	 value = _key

	 if((value == 0) or ((value < threshold) and (num > 2))) then break end
	 if(num > 0) then rsp = rsp .. " }," end
	 rsp = rsp .. '\n\t\t { "label": "'..key..'", "value": '..value
	 num = num + 1

	 if(num == max_num_entries) then
	    break
	 end
      end
   end

   if(mode == nil) then
      if(num > 0) then rsp = rsp .. " }" end
      rsp = rsp .. "\n\t]\n"
      rsp = rsp .. "\n}\n"
   else
      rsp = rsp .. " }\n"
      rsp = rsp .. "\n]\n"
   end

   --print(rsp.."\n")
   return(rsp)
end

-- #####################################################

function getTopASs(ifname)
   max_num_entries = 10
   rsp = ""

   if(ifname == nil) then ifname = "any" end

   interface.find(ifname)
   hosts_stats = interface.getHostsInfo()

   _asn = {}
   total = 0

   for _key, value in pairs(hosts_stats) do
      key = hosts_stats[_key]["asn"]

      if(key == 0) then
	 key = key .." [Local/Unknown]"
      else
	 if(hosts_stats[_key]["asname"] ~= nil) then key = key .." ["..abbreviateString(hosts_stats[_key]["asname"], 10).."]" end
      end
      old = _asn[key]
      if(old == nil) then old = 0 end
      val = hosts_stats[_key]["bytes.sent"] + hosts_stats[_key]["bytes.rcvd"]
      total = total + val
      _asn[key] = old + val
   end


   asn = {}
   for _key, value in pairs(_asn) do
      asn[value] = _key
   end

   rsp = rsp .. "[\n"

   -- 10 %
   threshold = total / 10

   num = 0
   for _key, _value in pairsByKeys(asn, rev) do
      key   = asn[_key]
      value = _key

      if((value == 0) or ((value < threshold) and (num > 2))) then break end
      if(num > 0) then rsp = rsp .. " }," end
      rsp = rsp .. '\n\t\t { "label": "'..key..'", "value": '..value
      num = num + 1

      if(num == max_num_entries) then
	 break
      end
   end

   rsp = rsp .. " }\n"
   rsp = rsp .. "\n]\n"

   return(rsp)
end
