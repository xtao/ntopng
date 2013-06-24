--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')

ifname = _GET["if"]
interface.find("any")
hosts_stats = interface.getHosts()

tot = 0
_hosts_stats = {}
top_key = nil
top_value = 0
num = 0
for key, value in pairs(hosts_stats) do
   if(key == "255.255.255.255") then
      key = "Broadcast"
   end
   _hosts_stats[value] = key -- ntop.getResolvedAddress(key)
   if((top_value < value) or (top_key == nil)) then
     top_key = key
     top_value = value
   end
   tot = tot + value
end

-- Print up to this number of entries
max_num_entries = 10

-- Print entries whose value >= 5% of the total
threshold = (tot * 5) / 100

print "[\n"
num = 0
accumulate = 0
for key, value in pairsByKeys(_hosts_stats, rev) do
   if(key < threshold) then
      break
   end

   if(num > 0) then
      print ",\n"
   end

   print("\t { \"label\": \"" .. value .."\", \"value\": ".. key ..", \"url\": \"/lua/host_details.lua?host=".. value .."\" }")
   accumulate = accumulate + key
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
   print(",\n\t { \"label\": \"Other\", \"value\": ".. (tot-accumulate) .." }")
end

print "\n]"




