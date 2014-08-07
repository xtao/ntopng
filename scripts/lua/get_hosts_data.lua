--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"


sendHTTPHeader('text/html')

-- Table parameters
all = _GET["all"]
currentPage = _GET["currentPage"]
perPage     = _GET["perPage"]
sortColumn  = _GET["sortColumn"]
sortOrder   = _GET["sortOrder"]
protocol    = _GET["protocol"]

-- Host comparison parameters
mode        = _GET["mode"]
aggregation = _GET["aggregation"]
aggregated  = _GET["aggregated"]
tracked     = _GET["tracked"]
protocol    = _GET["protocol"]

-- Only for aggregations
client      = _GET["client"]

-- table_id = _GET["table"]

-- Get from redis the throughput type bps or pps
throughput_type = getThroughputType()

if ((sortColumn == nil) or (sortColumn == "column_"))then
  sortColumn = getDefaultTableSort("hosts")
else
  if ((aggregated == nil) and (sortColumn ~= "column_")
    and (sortColumn ~= "")) then  
      tablePreferences("sort_hosts",sortColumn) 
  end
end

if(sortOrder == nil) then
  sortOrder = getDefaultTableSortOrder("hosts")
else
  if ((aggregated == nil) and (sortColumn ~= "column_")
    and (sortColumn ~= "")) then  
    tablePreferences("sort_order_hosts",sortOrder) 
  end
end

if(currentPage == nil) then
   currentPage = 1
else
   currentPage = tonumber(currentPage)
end

if(perPage == nil) then
   perPage = getDefaultTableSize()
else
  perPage = tonumber(perPage)
  tablePreferences("rows_number",perPage)
end

if((aggregation ~= nil) or (aggregated ~= nil)) then aggregation = tonumber(aggregation) end
if(tracked ~= nil) then tracked = tonumber(tracked) else tracked = 0 end

if((mode == nil) or (mode == "")) then mode = "all" end

interface.find(ifname)

if((aggregation == nil) and (aggregated == nil)) then
   hosts_stats = interface.getHostsInfo()
else
   hosts_stats = interface.getAggregatedHostsInfo(tonumber(protocol), client)
   protocol = nil -- Not applicable
end

to_skip = (currentPage-1) * perPage

if (all ~= nil) then
  perPage = 0
  currentPage = 0
end

print ("{ \"currentPage\" : " .. currentPage .. ",\n \"data\" : [\n")
num = 0
total = 0

now = os.time()
vals = {}
num = 0

-- for k,v in pairs(hosts_stats) do io.write(k.."\n") end

for key, value in pairs(hosts_stats) do
   num = num + 1
   postfix = string.format("0.%04u", num)
   ok = true

   if((aggregation ~= nil) and (hosts_stats[key]["aggregation"] ~= nil)) then
      --io.write(hosts_stats[key]["aggregation"].." <> "..aggregation.."\n")
      -- io.write("'"..hosts_stats[key]["aggregation"].."' <> '"..aggregation.."'\n")
      if(tonumber(hosts_stats[key]["aggregation"]) ~= aggregation) then
	 ok = false
      else
	 if(aggregation == 2) then
	    if(hosts_stats[key]["tracked"] == true) then
	       t = 1
	    else
	       t = 0
	    end

	    if(t ~= tracked) then
	       ok = false
	    end
	 end
      end
   end

   if(not((mode == "all")
       or ((mode == "local") and (value["localhost"] == true))
    or ((mode == "remote") and (value["localhost"] ~= true)))) then
      ok = false
   end

   if(ok == true) then
      if(client ~= nil) then
	 ok = false

	 for k,v in pairs(hosts_stats[key]["contacts"]["client"]) do
	    --io.write(k.."\n")
	    if((ok == false) and (k == client)) then ok = true end
	 end

	 if(ok == false) then
	    for k,v in pairs(hosts_stats[key]["contacts"]["server"]) do
	       -- io.write(k.."\n")
	       if((ok == false) and (k == client)) then ok = true end
	    end
	 end
      else
	 ok = true
      end
   end

   if((protocol ~= nil) and (ok == true)) then
      info = interface.getHostInfo(key)

      if(info["ndpi"][protocol] == nil) then
	 ok = false
      end
   end

   if(ok) then
      --print("==>"..hosts_stats[key]["bytes.sent"].."[" .. sortColumn .. "]\n")

    if(hosts_stats[key]["name"] == nil) then
      if(hosts_stats[key]["ip"] ~= nil) then
         hosts_stats[key]["name"] = ntop.getResolvedAddress(hosts_stats[key]["ip"])
      else
         hosts_stats[key]["name"] = hosts_stats[key]["mac"]
      end
    end

   if(sortColumn == "column_ip") then
	 vals[key] = key
	 elseif(sortColumn == "column_name") then
	 vals[hosts_stats[key]["name"]..postfix] = key
	 elseif(sortColumn == "column_since") then
	 vals[(now-hosts_stats[key]["seen.first"])+postfix] = key
	 elseif(sortColumn == "column_alerts") then
	 vals[(now-hosts_stats[key]["num_alerts"])+postfix] = key
	 elseif(sortColumn == "column_family") then
	 vals[(now-hosts_stats[key]["family"])+postfix] = key
	 elseif(sortColumn == "column_last") then
	 vals[(now-hosts_stats[key]["seen.last"]+1)+postfix] = key
	 elseif(sortColumn == "column_category") then
	 if(hosts_stats[key]["category"] == nil) then hosts_stats[key]["category"] = "" end
	 vals[hosts_stats[key]["category"]..postfix] = key
         elseif(sortColumn == "column_httpbl") then
	 if(hosts_stats[key]["httpbl"] == nil) then hosts_stats[key]["httpbl"] = "" end
	 vals[hosts_stats[key]["httpbl"]..postfix] = key
	 elseif(sortColumn == "column_asn") then
	 vals[hosts_stats[key]["asn"]..postfix] = key
	 elseif(sortColumn == "column_aggregation") then
	 vals[hosts_stats[key]["aggregation"]..postfix] = key
	 elseif(sortColumn == "column_vlan") then
	 vals[hosts_stats[key]["vlan"]..postfix] = key
	 elseif(sortColumn == "column_thpt") then
	 vals[hosts_stats[key]["throughput_"..throughput_type]+postfix] = key
	 elseif(sortColumn == "column_queries") then
	 vals[hosts_stats[key]["queries.rcvd"]+postfix] = key
      else
	 -- io.write(key.."\n")
	 -- io.write(hosts_stats[key].."\n")
	 -- for k,v in pairs(hosts_stats[key]) do io.write(k.."\n") end

	 vals[(hosts_stats[key]["bytes.sent"]+hosts_stats[key]["bytes.rcvd"])+postfix] = key
      end
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
	 if((num < perPage) or (all ~= nil))then
	    if(num > 0) then
	       print ",\n"
	    end
      print ('{ ')
      print ('\"key\" : \"'..hostinfo2jqueryid(hosts_stats[key])..'\",')
	    print ("\"column_ip\" : \"<A HREF='/lua/")
	    if((aggregation ~= nil) or (aggregated ~= nil)) then print("aggregated_") end
	    print("host_details.lua?" ..hostinfo2url(hosts_stats[key]) .. "'>")
	    if((aggregation == nil) and (aggregated == nil)) then
         if (value["ip"] ~= nil) then
          print(value["ip"])
        else
          print(value["mac"])
        end
	    else
	       print(mapOS2Icon(key))
	    end

	    print(" </A> ")

	    if((aggregation ~= nil) or (aggregated ~= nil)) then
	       --if(value["tracked"]) then

	       -- EPP + domain + tracked
	       if((value["aggregation"] == 2) and (value["family"] == 38)) then
		  if(value["tracked"]) then
		     print("<i class=\'fa fa-star fa-lg\'></i>")
		  else
		     print("<i class=\'fa fa-star-o fa-lg\'></i>")
		  end
	       end
	    end

	    print(getOSIcon(value["os"]).. "\", \"column_name\" : \"")

	    if((aggregation == nil) and (aggregated == nil)) then
	       if(value["name"] == nil) then value["name"] = ntop.getResolvedAddress(key) end
	    end
	    print(shortHostName(value["name"]))

	    if((value["alternate_name"] ~= nil) and (value["alternate_name"] ~= "")) then
	       print (" ["..value["alternate_name"].."]")
	    end

	    if((value["httpbl"] ~= nil) and (string.len(value["httpbl"]) > 2)) then
		  print (" <i class='fa fa-frown-o'></i>")
	       end

	    if((value["num_alerts"] ~= nil) and (value["num_alerts"] > 0)) then
	       print(" <i class='fa fa-warning fa-lg' style='color: #B94A48;'></i>")
	    end

	    --   print("</div>")

	    if(value["systemhost"] == true) then print("&nbsp;<i class='fa fa-flag'></i>") end
	    if(value["country"] ~= nil) then
	       print("&nbsp;<img src='/img/blank.gif' class='flag flag-".. string.lower(value["country"]) .."'>")
	    end

	    if(value["category"] ~= nil) then print("\", \"column_category\" : \"".. getCategory(value["category"])) end
	    if((value["httpbl"] ~= nil) and (string.len(value["httpbl"]) > 2)) then print("\", \"column_httpbl\" : \"".. value["httpbl"]) end

	    if (value["vlan"] ~= nil) then

        if (value["vlan"] ~= 0) then
          print("\", \"column_vlan\" : "..value["vlan"])
        else
          print("\", \"column_vlan\" : \"0\"")
        end

      else
	       print("\", \"column_vlan\" : \"\"")
	    end

	    if(value["asn"] ~= nil) then
	       if(value["asn"] == 0) then
		  print(", \"column_asn\" : 0")
	       else
		  print(", \"column_asn\" : \"" .. printASN(value["asn"], value["asname"]) .."\"")
	       end
	    end

	    if((aggregation ~= nil) or (aggregated ~= nil)) then
	       print(", \"column_family\" : \"" .. interface.getNdpiProtoName(value["family"]) .. "\"")
	       print(", \"column_aggregation\" : \"" .. aggregation2String(value["aggregation"]) .. "\"")
         throughput_type = "bps"
	    end

	    print(", \"column_since\" : \"" .. secondsToTime(now-value["seen.first"]+1) .. "\", ")
	    print("\"column_last\" : \"" .. secondsToTime(now-value["seen.last"]+1) .. "\", ")

      if (  (value["throughput_trend_"..throughput_type] ~= nil) and
            (value["throughput_trend_"..throughput_type] > 0)
      ) then

        if (throughput_type == "pps") then
          print ("\"column_thpt\" : \"" .. pktsToSize(value["throughput_bps"]).. " ")
        else
          print ("\"column_thpt\" : \"" .. bitsToSize(8*value["throughput_bps"]).. " ")
        end

        if(value["throughput_trend_"..throughput_type] == 1) then
          print("<i class='fa fa-arrow-up'></i>")
        elseif(value["throughput_trend_"..throughput_type] == 2) then
          print("<i class='fa fa-arrow-down'></i>")
        elseif(value["throughput_trend_"..throughput_type] == 3) then
          print("<i class='fa fa-minus'></i>")
        end

        print("\",")
      else
        print ("\"column_thpt\" : \"0 "..throughput_type.."\",")
      end

      if((aggregation ~= nil) or (aggregated ~= nil)) then
	       --print("\"column_traffic\" : \"" .. formatValue(value["bytes.sent"]+value["bytes.rcvd"]).." ")
	       print("\"column_queries\" : \"" .. formatValue(value["queries.rcvd"]).." ")

	       if(value["throughput_trend_"..throughput_type] == 1) then
          print("<i class='fa fa-arrow-up'></i>")
        elseif(value["throughput_trend_"..throughput_type] == 2) then
          print("<i class='fa fa-arrow-down'></i>")
        elseif(value["throughput_trend_"..throughput_type] == 3) then
          print("<i class='fa fa-minus'></i>")
        end
	    else
	       print("\"column_traffic\" : \"" .. bytesToSize(value["bytes.sent"]+value["bytes.rcvd"]))
	    end

	    print ("\", \"column_alerts\" : \"")
	    if((value["num_alerts"] ~= nil) and (value["num_alerts"] > 0)) then
	       print(""..value["num_alerts"])
	    else
	       print("0")
	    end
	    if(value["localhost"] ~= nil) then
	       print ("\", \"column_location\" : \"")
	       if(value["localhost"] == true) then print("<span class='label label-success'>Local</span>") else print("<span class='label label-default'>Remote</span>") end
	    end

	    sent2rcvd = round((value["bytes.sent"] * 100) / (value["bytes.sent"]+value["bytes.rcvd"]), 0)
	    print ("\", \"column_breakdown\" : \"<div class='progress'><div class='progress-bar progress-bar-warning' style='width: "
		   .. sent2rcvd .."%;'>Sent</div><div class='progress-bar progress-bar-info' style='width: " .. (100-sent2rcvd) .. "%;'>Rcvd</div></div>")

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
