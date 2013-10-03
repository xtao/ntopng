--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"

sendHTTPHeader('text/html')

currentPage     = _GET["currentPage"]
perPage         = _GET["perPage"]
sortColumn      = _GET["sortColumn"]
sortOrder       = _GET["sortOrder"]
aggregated      = _GET["aggregated"]
protocol        = _GET["protocol"]

if(sortColumn == nil) then
   sortColumn = "column_"
end

if(currentPage == nil) then
   currentPage = 1
else
   currentPage = tonumber(currentPage)
end

if(perPage == nil) then
   perPage = 5
else
   perPage = tonumber(perPage)
end

interface.find(ifname)

if(aggregated == nil) then
   hosts_stats = interface.getHostsInfo()
else
   hosts_stats = interface.getAggregatedHostsInfo(tonumber(protocol))
end

print ("{ \"currentPage\" : " .. currentPage .. ",\n \"data\" : [\n")
num = 0
total = 0
to_skip = (currentPage-1) * perPage

now = os.time()
vals = {}
num = 0

--for k,v in pairs(hosts_stats) do io.write(k.."\n") end

for key, value in pairs(hosts_stats) do
   num = num + 1
   postfix = string.format("0.%04u", num)

   --print("==>"..hosts_stats[key]["bytes.sent"].."[" .. sortColumn .. "]\n")
   if(sortColumn == "column_ip") then
      vals[key] = key
      elseif(sortColumn == "column_name") then
      if(hosts_stats[key]["name"] == nil) then 
	 if(hosts_stats[key]["ip"] ~= nil) then
	    hosts_stats[key]["name"] = ntop.getResolvedAddress(hosts_stats[key]["ip"]) 
	 else
	    hosts_stats[key]["name"] = hosts_stats[key]["mac"]
	 end
      end
      vals[hosts_stats[key]["name"]..postfix] = key
      elseif(sortColumn == "column_since") then
      vals[(now-hosts_stats[key]["seen.first"])+postfix] = key
      elseif(sortColumn == "column_last") then
      vals[(now-hosts_stats[key]["seen.last"]+1)+postfix] = key
      elseif(sortColumn == "column_category") then
      vals[hosts_stats[key]["category"]..postfix] = key
      elseif(sortColumn == "column_asn") then
      vals[hosts_stats[key]["asn"]..postfix] = key
      elseif(sortColumn == "column_thpt") then
      vals[hosts_stats[key]["throughput"]+postfix] = key
   else
      -- io.write(key.."\n")
      -- io.write(hosts_stats[key].."\n")
      -- for k,v in pairs(hosts_stats[key]) do io.write(k.."\n") end

      vals[(hosts_stats[key]["bytes.sent"]+hosts_stats[key]["bytes.rcvd"])+postfix] = key
   end
end

table.sort(vals)

if(sortOrder == "asc") then
   funct = asc
else
   funct = rev
end

num = 0
for _key, _value in pairsByKeys(vals, funct) do
   key = vals[_key]

   if((key ~= nil) and (not(key == ""))) then
      value = hosts_stats[key]

      if(to_skip > 0) then
	 to_skip = to_skip-1
      else
	 if(num < perPage) then
	    if(num > 0) then
	       print ",\n"
	    end

	    print ("{ \"column_ip\" : \"<A HREF='/lua/")
	    if(aggregated ~= nil) then print("aggregated_") end
	    print("host_details.lua?host=" .. key .. "'>" .. key .. " ")

	    print("</A>\", \"column_name\" : \"")
	    if(value["name"] == nil) then value["name"] = ntop.getResolvedAddress(key) end
	    print(value["name"])
	    print("</div>")

	    if(value["country"] ~= nil) then
	       print("&nbsp;<img src='/img/blank.gif' class='flag flag-".. string.lower(value["country"]) .."'>")
	    end

	    if(value["category"] ~= nil) then print("\", \"column_category\" : \"".. getCategory(value["category"])) end
	    if(value["vlan"] ~= nil) then print("\", \"column_vlan\" : "..value["vlan"]) end

	    if(value["asn"] ~= nil) then
	       if(value["asn"] == 0) then
		  print(", \"column_asn\" : 0")
	       else
		  print(", \"column_asn\" : \"" .. printASN(value["asn"], value["asname"]) .."\"")
	       end
	    else
	       print("\"")
	    end

	    if(aggregated ~= nil) then 
	       print(", \"column_family\" : \"" .. value["family"] .. "\"")
	    end

	    print(", \"column_since\" : \"" .. secondsToTime(now-value["seen.first"]+1) .. "\", ")
	    print("\"column_last\" : \"" .. secondsToTime(now-value["seen.last"]+1) .. "\", ")

	    if(value["throughput_trend"] > 0) then
	       print ("\"column_thpt\" : \"" .. bitsToSize(8*value["throughput"]).. " ")
	       print("\",")
	    end

	    if(aggregated ~= nil) then
	       print("\"column_traffic\" : \"" .. formatValue(value["bytes.sent"]+value["bytes.rcvd"]).." ")

	       if(value["throughput_trend"] == 1) then
		  print("<i class=icon-arrow-up></i>")
		  elseif(value["throughput_trend"] == 2) then
		  print("<i class=icon-arrow-down></i>")
		  elseif(value["throughput_trend"] == 3) then
		  print("<i class=icon-minus></i>")
	       end
	    else
	       print("\"column_traffic\" : \"" .. bytesToSize(value["bytes.sent"]+value["bytes.rcvd"]))
	    end

	    if(value["localhost"] ~= nil) then
	       print ("\", \"column_location\" : \"")
	       if(value["localhost"] == true) then print("<span class='label label-success'>Local</span>") else print("<span class='label'>Remote</span>") end
	    end

	    sent2rcvd = round((value["bytes.sent"] * 100) / (value["bytes.sent"]+value["bytes.rcvd"]), 0)
	    print ("\", \"column_breakdown\" : \"<div class='progress'><div class='bar bar-warning' style='width: " .. sent2rcvd .."%;'>Sent</div><div class='bar bar-info' style='width: " .. (100-sent2rcvd) .. "%;'>Rcvd</div></div>")


	    print("\" } ")
	    num = num + 1
	 end
      end

      total = total + 1
   end
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
