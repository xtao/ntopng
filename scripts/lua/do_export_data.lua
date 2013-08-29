--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('application/json')


interface.find(ifname)
if((_GET["hostIP"] ~= nil) and (_GET["hostIP"] ~= "")) then
   host = interface.getHostInfo(_GET["hostIP"])

   if(host == nil) then 
      print("{ }\n")
   else
      print(host["json"].."\n")
   end
else
   -- All hosts
   hosts_stats = interface.getHostsInfo()
   num = 0
   print("[\n")

   for key, value in pairs(hosts_stats) do
      host = interface.getHostInfo(key)
      
      if(host["json"] ~= nil) then
	 if(num > 0) then print(",\n") end
	 print(host["json"])
	 num = num + 1
      end
   end

   print("\n]\n")
end