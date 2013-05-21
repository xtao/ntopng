package.path = "./scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"

ifname          = _GET["if"]
currentPage     = _GET["currentPage"]
perPage         = _GET["perPage"]
sortColumn      = _GET["sortColumn"]
sortOrder       = _GET["sortOrder"]


if(currentPage == nil) then
   currentPage = 1
else
   currentPage = tonumber(currentPage)
end

if(perPage == nil) then
   perPage = 1
else
   perPage = tonumber(perPage)
end


if(ifname == nil) then	  
  ifname = "any"
end

interface.find(ifname)
hosts_stats = interface.getHostsInfo()

print ("{ \"currentPage\" : " .. currentPage .. ",\n \"data\" : [\n")
num = 0
total = 0
to_skip = (currentPage-1) * perPage

vals = {}
for key, value in pairs(hosts_stats) do
--   print("==>"..hosts_stats[key]["bytes.sent"].."\n")
   if(sortColumn == "column_0") then
      vals[key] = key
   elseif(sortColumn == "column_1") then
   vals[hosts_stats[key]["name"]] = key
   elseif(sortColumn == "column_2") then
   vals[hosts_stats[key]["bytes.sent"]+hosts_stats[key]["bytes.rcvd"]] = key
else
   vals[hosts_stats[key]["bytes.sent"]+hosts_stats[key]["bytes.rcvd"]] = key
   end
end

table.sort(vals)

if(sortOrder == "asc") then
   funct = asc
else
   funct = rev
end

for _key, _value in pairsByKeys(vals, funct) do
   key = vals[_key]   
   value = hosts_stats[key]

   if(to_skip > 0) then
      to_skip = to_skip-1
   else
      if(num < perPage) then
	 if(num > 0) then
	    print ",\n"
	 end
	 
	 print ("{ \"column_ip\" : \"<A HREF='/host_details.lua?interface=".. ifname .. "&host=" .. key .. "'>" .. key .. "</A>\", \"column_name\" : \"" .. value["name"] .. "\", \"column_since\" : \"" .. secondsToTime(value["duration"]) .. "\", \"column_traffic\" : \"" .. bytesToSize(value["bytes.sent"]+value["bytes.rcvd"]))


	 sent2rcvd = round((value["bytes.sent"] * 100) / (value["bytes.sent"]+value["bytes.rcvd"]), 0)
	 print ("\", \"column_breakdown\" : \"<div class='progress'><div class='bar bar-warning' style='width: " .. sent2rcvd .."%;'></div><div class='bar bar-info' style='width: " .. (100-sent2rcvd) .. "%;'></div></div>")


	 print("\" } ")
	 num = num + 1
      end
   end

   total = total + 1
end -- for


print ("\n], \"perPage\" : " .. perPage .. ",\n")

if(sortColumn == nil) then
   sortColumn = ""
end

if(sortOrder == nil) then
   sortOrder = ""
end

print ("\"sort\" : [ [ \"" .. sortColumn .. "\", \"" .. sortOrder .."\" ] ],\n")

print ("\"totalRows\" : " .. total .. " \n}")
