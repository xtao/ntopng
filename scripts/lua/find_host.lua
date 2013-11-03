--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('application/json')

print [[
{
    "results": [
]]

   interface.find(ifname)
   hosts_stats = interface.getHostsInfo()
   num = 0
   query = _GET["query"]
--   query = "192"

   if(query ~= nil) then
      query = string.lower(query)

   for _key, value in pairs(hosts_stats) do
      found = 0
      if((hosts_stats[_key]["name"] == nil) and (hosts_stats[_key]["ip"] ~= nil)) then
	 hosts_stats[_key]["name"] = ntop.getResolvedAddress(hosts_stats[_key]["ip"])
      end
      what = hosts_stats[_key]["name"]

      if((what ~= nil) and (starts(string.lower(what), query))) then
	 found = 1
      else
	 what = hosts_stats[_key]["mac"]
	 if(starts(what, query)) then
	    found = 1
	 else
	    if(hosts_stats[_key]["ip"] ~= nil) then
	       what = hosts_stats[_key]["ip"]
	       if(starts(what, query)) then
		  found = 1
	       end
	    end
	 end
      end
      
      if(found == 1) then
	 if(num > 0) then print(",\n") end
	 print("\t\""..what .. "\"")
	 num = num + 1
      end
   end


   aggregated_hosts_stats = interface.getAggregatedHostsInfo()
   for _key, value in pairs(aggregated_hosts_stats) do
      found = 0
      if((aggregated_hosts_stats[_key]["name"] == nil) and (aggregated_hosts_stats[_key]["ip"] ~= nil)) then
	 aggregated_hosts_stats[_key]["name"] = ntop.getResolvedAddress(aggregated_hosts_stats[_key]["ip"])
      end
      what = aggregated_hosts_stats[_key]["name"]
      if((what ~= nil) and (starts(what, query))) then
	 found = 1
	 what = what .. " (" .. aggregated_hosts_stats[_key]["family"] .. ")"
      end

      if(found == 1) then
	 if(num > 0) then print(",\n") end
	 print("\t\""..what .. "\"")
	 num = num + 1
      end
   end
end

print [[

    ]
}
]]

