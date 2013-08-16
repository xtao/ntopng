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

   for _key, value in pairs(hosts_stats) do
      found = 0 
      what = hosts_stats[_key]["name"]
      if(starts(what, query)) then 
	 found = 1
      else
	 what = hosts_stats[_key]["mac"]
	 if(starts(what, query)) then 
            found = 1
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
      what = aggregated_hosts_stats[_key]["name"]
      if(starts(what, query)) then 
	 found = 1
      end
      
      if(found == 1) then
	 if(num > 0) then print(",\n") end
	 print("\t\""..what .. "\"")
	 num = num + 1
      end
   end
   
print [[

    ]
}
]]

