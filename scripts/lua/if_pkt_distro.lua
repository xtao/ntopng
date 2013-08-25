--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')

interface.find(ifname)
ifstats = interface.getStats()

type = _GET["type"]

if((type == nil) or (type == "size")) then
   what = ifstats["pktSizeDistribution"]
end



print "[\n"
num = 0
for key, value in pairs(what) do
   if(value > 0) then
      if(num > 0) then
	 print ",\n"
      end
   
      print("\t { \"label\": \"" .. key .."\", \"value\": ".. value .." }") 
      num = num + 1
   end
end


print "\n]"

