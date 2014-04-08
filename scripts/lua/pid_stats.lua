--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')

mode = _GET["mode"] --l4,l7,host
pid = tonumber(_GET["pid"])
name = _GET["name"]
host = _GET["host"]
local debug = false

interface.find(ifname)

if (pid ~= nil) then
   flows = interface.findPidFlows(pid)
elseif (name ~= nil) then
   flows = interface.findNameFlows(name)
end

if(mode == nil) then
   mode = "host"
end

local debug = false

if(flows == nil) then
   print('[ { "label": "No flows", "value": 1 } ]') -- No flows found
else   
   apps = { }
   tot = 0
   for k,f in pairs(flows) do 
      process = 1
      if (debug) then io.write("Cli:"..f["cli.ip"].." - Srv:"..f["srv.ip"].."\n") end
      if(debug) then print(k.."\n") end
      if((host ~= nil) and ((f["cli.ip"] ~= host) and (f["srv.ip"] ~= host))) then
         process = 0
      end

      if(mode == "l7") then
        key = f["proto.ndpi"]
        v = f["cli2srv.bytes"] + f["srv2cli.bytes"]
      elseif(mode == "l4") then
        key = f["proto.l4"]
	if(debug) then print(key.."\n") end
        v = f["cli2srv.bytes"] + f["srv2cli.bytes"]
      elseif(mode == "host") then
        if ((f["client_process"] ~= nil) and (f["client_process"]["name"] == name)) then
          key = f["cli.source_id"].."-"..f["cli.ip"].."(client)"
          v = f["cli2srv.bytes"] 
        elseif ((f["server_process"] ~= nil) and (f["server_process"]["name"] == name)) then
          key = f["srv.source_id"].."-"..f["srv.ip"].."(server)"
          v = f["srv2cli.bytes"]
        end
      end
      
      if((key ~= nil) and (process == 1))then
         if(apps[key] == nil) then apps[key] = 0 end
         apps[key] = apps[key] + v
         tot = tot + v
      end
   end

-- Print up to this number of entries
max_num_entries = 10

-- Print entries whose value >= 5% of the total
threshold = (tot * 5) / 100

print "[\n"
num = 0
accumulate = 0

for key, value in pairs(apps) do
   if((num ~= 0) and (value < threshold)) then
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



