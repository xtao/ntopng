--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')

interface.find(ifname)

mode = _GET["mode"]
type = _GET["type"]
host = interface.getHostInfo(_GET["host"])

if((type == nil) or (type == "size")) then
   if((mode == nil) or (mode == "sent")) then
      what = host["pktStats.sent"]
   else
      what = host["pktStats.recv"]
   end
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

