--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "persistence"

function getTopTalkers(ifid, ifname, mode, epoch)
   -- if(ifname == nil) then ifname = "any" end

   if(epoch ~= nil) then
      rsp = getHistoricalTopTalkers(ifid, ifname, mode, epoch)
   else
      rsp = getActualTopTalkers(ifid, ifname, mode)
   end

   return(rsp)
end


-- #################################################

function getHistoricalTopTalkers(ifid, ifname, mode, epoch)
   epoch = epoch - (epoch % 60)
   dirs = ntop.getDirs()
   filename = fixPath(dirs.workingdir .. "/".. ifid .. "/top_talkers/" .. os.date("%Y/%m/%d/%H", epoch) .. os.date("/%M.json", epoch))

   --print(filename)
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

function getTop(stats, sort_field_key, max_num_entries, lastdump_dir)
   local _filtered_stats, filtered_stats, counter, total,
         threshold, low_threshold

   -- stats is a hash of hashes organized as follows:
   -- { "id1" : { "key1": "value1", "key2": "value2", ...}, "id2 : { ... } }
   -- filter out the needed values
   _filtered_stats = {}
   for id,content in pairs(stats) do
      _filtered_stats[id] = content[sort_field_key]
   end

   -- Read the lastdump; the name of the lastdump file has the following
   -- format: <lastdump_dir>/.<sort_field_key>_lastdump
   lastdump = lastdump_dir .. "/."..sort_field_key.."_lastdump"
   last = nil
   if(ntop.exists(lastdump)) then
      last = persistence.load(lastdump)
   end
   if(last == nil) then last = {} end

   persistence.store(lastdump, _filtered_stats);

   for key, value in pairs(_filtered_stats) do
      if(last[key] ~= nil) then
         v = _filtered_stats[key]-last[key]
         if(v < 0) then v = 0 end
         _filtered_stats[key] = v
      end
   end

   -- order the filtered stats by using the value (bytes sent/received during
   -- the last time interval) as key
   filtered_stats = {}
   for key, value in pairs(_filtered_stats) do
      filtered_stats[value] = key
   end

   -- Compute traffic
   total = 0
   for _value,_ in pairsByKeys(filtered_stats, rev) do
      total = total + _value
   end

   threshold = total / 10 -- 10 %
   low_threshold = total * 0.05  -- 5%

   -- build a new hashtable sorted by the required field
   top_stats = {}
   counter = 0
   for _value,_id in pairsByKeys(filtered_stats, rev) do
      if ((_value == 0) or (_value < low_threshold) or
          ((_value < threshold) and (counter > max_num_entries / 2))) then
         break
      end
      top_stats[_value] = _id -- still keep it in order
      counter = counter + 1
      if (counter == max_num_entries) then
        break
      end
   end

   return top_stats
end

-- #####################################################

function getActualTopTalkers(ifid, ifname, mode, epoch)
   max_num_entries = 10
   rsp = ""

   interface.find(ifname)
   hosts_stats = interface.getHostsInfo()

   talkers_dir = fixPath(dirs.workingdir .. "/" .. ifid .. "/top_talkers")
   if(not(ntop.exists(talkers_dir))) then
      ntop.mkdir(talkers_dir)
   end

   if(mode == nil) then
      rsp = rsp .. "{\n"
      rsp = rsp .. '\t"senders": ['
   else
      rsp = rsp .. "[\n"
   end

   --print("Hello\n")
   if((mode == nil) or (mode == "senders")) then
      top_senders = getTop(hosts_stats, "bytes.sent", max_num_entries, talkers_dir)
      num = 0
      for value,id in pairsByKeys(top_senders, rev) do
	 if(num > 0) then rsp = rsp .. " }," end
	 rsp = rsp .. '\n\t\t { "label": "'..id.. '", "url": "'
               ..ntop.getHttpPrefix()..
               '/lua/host_details.lua?host='..id..'", "value": '..value
	 num = num + 1
      end
   end

   if(mode == nil) then
      if(num > 0) then rsp = rsp .. " }" end
      rsp = rsp .. "\n\t],\n"
      rsp = rsp .. '\t"receivers": ['
   end

   if((mode == nil) or (mode == "receivers")) then
      top_receivers = getTop(hosts_stats, "bytes.rcvd", max_num_entries, talkers_dir)
      num = 0
      for value,id in pairsByKeys(top_receivers, rev) do
	 if(num > 0) then rsp = rsp .. " }," end
	 rsp = rsp .. '\n\t\t { "label": "'..id.. '", "url": "'
               ..ntop.getHttpPrefix()..
               '/lua/host_details.lua?host='..id..'", "value": '..value
	 num = num + 1
      end
   end

   if(mode == nil) then
      if(num > 0) then rsp = rsp .. " }" end
      rsp = rsp .. "\n\t]\n"
      rsp = rsp .. "\n}\n"
   else
       if(num > 0) then rsp = rsp .. " }\n" end
      rsp = rsp .. "\n]\n"
   end

   --print(rsp.."\n")
   return(rsp)
end

-- #####################################################

function getActualTopXASs(ifname, max_num_entries, use_threshold, use_delta)
   rsp = ""

   --if(ifname == nil) then ifname = "any" end

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
   low_threshold = total * 0.050

   num = 0
   for _key, _value in pairsByKeys(asn, rev) do
      key   = asn[_key]
      value = _key

      if(value == 0) then break end
      
      if(use_threshold) then
	 if((value < low_threshold) or ((value < threshold) and (num > 2))) then break end
      end

      if(num > 0) then rsp = rsp .. " }," end
      k = string.split(key, " ")
      rsp = rsp .. '\n\t\t { "asn": "'..k[1].. '", "label": "'..key..'", "value": '..value
      num = num + 1

      if(num == max_num_entries) then break end
   end

   rsp = rsp .. " }\n"
   rsp = rsp .. "\n]\n"

   return(rsp)
end

function getTopASs(ifname)
   return(getActualTopXASs(ifname, 10, true, false))
end

