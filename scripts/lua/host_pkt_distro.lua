--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')



mode = _GET["mode"]
type = _GET["type"]
host = _GET["host"]

interface.find(ifname)


if(host == nil) then
   print("<div class=\"alert alert-error\"><img src=/img/warning.png> This flow cannot be found (expired ?)</div>")
else

flows_stats = interface.getFlowsInfo()

if((type == nil) or (type == "memory")) then
   how = "process.actual_memory"
elseif (type == "bytes") then
   how = "bytes"
end

if((mode == nil) or (mode == "user")) then
   what = "process.user_name"
else
   what = "process.name"
end

tot = 0
for key, value in pairs(flows_stats) do
   tot = tot + flows_stats[key][how]
end

threshold = (5 * tot) / 100

print "[\n"
num = 0
s = 0
for key, value in pairs(flows_stats) do
   if(flows_stats[key][how] > threshold) then
      if(num > 0) then
	 print ",\n"
      end
   
      print("\t { \"label\": \"" .. flows_stats[key][what] .."\", \"value\": ".. flows_stats[key][how] .." }") 
      num = num + 1
      s = s + flows_stats[key][how]
   end
end

if(tot > s) then
   print(",\t { \"label\": \"Other\", \"value\": ".. (tot-s) .." }") 
end


print "\n]"

end