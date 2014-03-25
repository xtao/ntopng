--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')

mode = _GET["mode"]
pid = tonumber(_GET["pid"])
name = _GET["name"]


interface.find(ifname)

if (pid ~= nil) then
   flows = interface.findPidFlows(pid)
elseif (name ~= nil) then
   flows = interface.findNameFlows(name)
end

if(flows == nil) then
   print('[ { "label": "Other", "value": 1 } ]') -- No flows found
else   
   apps = { }
   tot = 0
   for k,f in pairs(flows) do       
      if(mode == "l7") then
	     key = f["proto.ndpi"]
      end
      if(mode == "l4") then
         key = f["proto.l4"]
      end
      
      if(apps[key] == nil) then apps[key] = 0 end
      v = f["cli2srv.bytes"] + f["srv2cli.bytes"]
      apps[key] = apps[key] + v
      tot = tot + v
   end

-- Print up to this number of entries
max_num_entries = 10

-- Print entries whose value >= 5% of the total
threshold = (tot * 5) / 100

print "[\n"
num = 0
accumulate = 0
for key, value in pairs(apps) do
   if(value < threshold) then
      break
   end

   if(num > 0) then
      print ",\n"
   end

   print("\t { \"label\": \"" .. key .."\", \"value\": ".. value .." }")
   accumulate = accumulate + value
   num = num + 1

   if(num == max_num_entries) then
      break
   end
end

if((num == 0) and (top_key ~= nil)) then
   print("\t { \"label\": \"" .. top_key .."\", \"value\": ".. top_value ..", \"url\": \"/lua/host_details.lua?host=".. top_key .."\" }")
   accumulate = accumulate + top_value
end

-- In case there is some leftover do print it as "Other"
if(accumulate < tot) then
   if(num > 0) then print(",") end 
   print("\n\t { \"label\": \"Other\", \"value\": ".. (tot-accumulate) .." }")
end

print "\n]"
end



