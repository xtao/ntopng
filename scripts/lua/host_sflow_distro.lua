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
local debug = false

if(host == nil) then
   print("<div class=\"alert alert-error\"><img src=/img/warning.png> This flow cannot be found (expired ?)</div>")
else

   flows_stats = interface.getFlowsInfo()

   how_is_process = 0

   if((type == nil) or (type == "memory")) then
      how = "actual_memory"
      how_is_process = 1
      elseif (type == "bytes") then
         how = "bytes"
      end

      if((mode == nil) or (mode == "user")) then
         what = "user_name"
         url = "/lua/get_user_info.lua?user="
         elseif (mode == "process") then
            what = "name"
            url = "/lua/get_process_info.lua?name="
         end

         tot = 0
         what_array = {}
         num = 0
         for key, value in pairs(flows_stats) do
            current_what = flows_stats[key]["process"][what]
   -- if (current_what == "") then current_what = "Unknown" end
   if (how_is_process == 1) then
      value = flows_stats[key]["process"][how] 
   else
      value = flows_stats[key][how]
   end
   if (what_array[current_what] == nil) then what_array[current_what]  = 0 end
   what_array[current_what] =  what_array[current_what] + value
   tot = tot + value
   

end

print "[\n"
num = 0
s = 0
for key, value in pairs(what_array) do

   if(num > 0) then
     print ",\n"
  end


  print("\t { \"label\": \"" .. key .."\", \"value\": ".. value ..", \"url\": \"" .. url..key.."\" }") 
  num = num + 1
  s = s + value
  
end

if(tot > s) then
   print(",\t { \"label\": \"Other\", \"value\": ".. (tot-s) .." }") 
end


print "\n]"

end