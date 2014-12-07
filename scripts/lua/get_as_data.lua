--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"

sendHTTPHeader('text/html; charset=iso-8859-1')

-- Table parameters
all = _GET["all"]
currentPage = _GET["currentPage"]
perPage     = _GET["perPage"]
sortColumn  = _GET["sortColumn"]
sortOrder   = _GET["sortOrder"]

as_n        = _GET["as"]

-- Get from redis the throughput type bps or pps
throughput_type = getThroughputType()

if ((sortColumn == nil) or (sortColumn == "column_"))then
  sortColumn = getDefaultTableSort("asn")
else
  if ((aggregated == nil) and (sortColumn ~= "column_")
    and (sortColumn ~= "")) then
      tablePreferences("sort_asn",sortColumn)
  end
end

if(sortOrder == nil) then
  sortOrder = getDefaultTableSortOrder("asn")
else
  if ((aggregated == nil) and (sortColumn ~= "column_")
    and (sortColumn ~= "")) then
    tablePreferences("sort_order_asn",sortOrder)
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

interface.find(ifname)
hosts_stats = interface.getHostsInfo()

to_skip = (currentPage-1) * perPage

if (all ~= nil) then
  perPage = 0
  currentPage = 0
end

if (as_n == nil) then -- single AS info requested
   print ("{ \"currentPage\" : " .. currentPage .. ",\n \"data\" : [\n")
end
num = 0
total = 0

now = os.time()
vals = {}

stats_by_as = {}
for key,value in pairs(hosts_stats) do
   -- Convert AS code to string to avoid type mismatches if the value is 0
   -- (which would mean that the AS is private)
   value["asn"] = tostring(value["asn"])

   asn = value["asn"]
   existing = stats_by_as[asn]
   if (existing == nil) then
      stats_by_as[asn] = {}
      stats_by_as[asn]["id"] = asn
      if (asn ~= "0") then
         stats_by_as[asn]["name"] = value["asname"]
      else
         stats_by_as[asn]["name"] = "Private ASN"
      end
      stats_by_as[asn]["seen.first"] = value["seen.first"]
      stats_by_as[asn]["seen.last"] = value["seen.last"]
   else
      stats_by_as[asn]["seen.first"] =
         math.min(stats_by_as[asn]["seen.first"], value["seen.first"])
      stats_by_as[asn]["seen.last"] =
         math.max(stats_by_as[asn]["seen.last"], value["seen.last"])
   end
   stats_by_as[asn]["num_hosts"] = 1 +
         ternary(existing, stats_by_as[asn]["num_hosts"], 0)
   stats_by_as[asn]["num_alerts"] = value["num_alerts"] +
         ternary(existing, stats_by_as[asn]["num_alerts"], 0)
   stats_by_as[asn]["throughput_bps"] = value["throughput_bps"] +
         ternary(existing, stats_by_as[asn]["throughput_bps"], 0)
   stats_by_as[asn]["throughput_pps"] = value["throughput_pps"] +
         ternary(existing, stats_by_as[asn]["throughput_pps"], 0)
   stats_by_as[asn]["throughput_trend_bps_diff"] =
         math.floor(value["throughput_trend_bps_diff"]) +
         ternary(existing, stats_by_as[asn]["throughput_trend_bps_diff"], 0)
   stats_by_as[asn]["bytes.sent"] = value["bytes.sent"] +
         ternary(existing, stats_by_as[asn]["bytes.sent"], 0)
   stats_by_as[asn]["bytes.rcvd"] = value["bytes.rcvd"] +
         ternary(existing, stats_by_as[asn]["bytes.rcvd"], 0)
   stats_by_as[asn]["country"] = value["country"]
end

function print_single_as(value)
   print ('{ ')
   print ('\"key\" : \"'..value["id"]..'\",')

   print ("\"column_asn\" : \"<A HREF='"..ntop.getHttpPrefix().."/lua/")
   print("hosts_stats.lua?asn=" ..value["id"] .. "'>")
   print(value["id"]..'</A>", ')

   print('"column_hosts" : "' .. formatValue(value["num_hosts"]) ..'",')

   print ("\"column_alerts\" : \"")
   if((value["num_alerts"] ~= nil) and (value["num_alerts"] > 0)) then
      print("<font color=#B94A48>"..formatValue(value["num_alerts"]).."</font>")
   else
      print("0")
   end
   print('", ')

   print("\"column_name\" : \""..value["name"])
   if((value["country"] ~= nil) and (value["country"] ~= "")) then
      print("&nbsp;<img src='/img/blank.gif' class='flag flag-".. string.lower(value["country"]) .."'>")
   end
   print('", ')

   print("\"column_since\" : \"" .. secondsToTime(now-value["seen.first"]+1) .. "\", ")

   sent2rcvd = round((value["bytes.sent"] * 100) / (value["bytes.sent"]+value["bytes.rcvd"]), 0)
   print ("\"column_breakdown\" : \"<div class='progress'><div class='progress-bar progress-bar-warning' style='width: "
          .. sent2rcvd .."%;'>Sent</div><div class='progress-bar progress-bar-info' style='width: "
          .. (100-sent2rcvd) .. "%;'>Rcvd</div></div>")
   print('", ')

   if (throughput_type == "pps") then
      print ("\"column_thpt\" : \"" .. pktsToSize(value["throughput_bps"]).. " ")
   else
      print ("\"column_thpt\" : \"" .. bitsToSize(8*value["throughput_bps"]).. " ")
   end
   if(value["throughput_trend_bps_diff"] > 0) then
      print("<i class='fa fa-arrow-up'></i>")
   elseif(value["throughput_trend_bps_diff"] < 0) then
      print("<i class='fa fa-arrow-down'></i>")
   else
      print("<i class='fa fa-minus'></i>")
   end
   print('", ')

   print("\"column_traffic\" : \"" .. bytesToSize(value["bytes.sent"]+value["bytes.rcvd"]))

   print("\" } ")
end

if (as_n ~= nil) then
   as_val = stats_by_as[as_n]
   if (as_val == nil) then
      print('{}')
   else
      print_single_as(as_val)
   end
   stats_by_as = {}
end

for key,value in pairs(stats_by_as) do
   if(sortColumn == "column_asn") then
      vals[key] = key
   elseif(sortColumn == "column_name") then
      vals[stats_by_as[key]["name"]] = key
   elseif(sortColumn == "column_since") then
      vals[(now-stats_by_as[key]["seen.first"])] = key
   elseif(sortColumn == "column_alerts") then
      vals[(now-stats_by_as[key]["num_alerts"])] = key
   elseif(sortColumn == "column_last") then
      vals[(now-stats_by_as[key]["seen.last"]+1)] = key
   elseif(sortColumn == "column_thpt") then
      vals[stats_by_as[key]["throughput_"..throughput_type]] = key
   elseif(sortColumn == "column_queries") then
      vals[stats_by_as[key]["queries.rcvd"]] = key
   else
      vals[(stats_by_as[key]["bytes.sent"]+stats_by_as[key]["bytes.rcvd"])] = key
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
      value = stats_by_as[key]

      if(to_skip > 0) then
         to_skip = to_skip-1
      else
         if((num < perPage) or (all ~= nil))then
            if(num > 0) then
               print ",\n"
            end
            print_single_as(value)
            num = num + 1
         end
      end
      total = total + 1
   end
end -- for

if (as_n == nil) then -- single AS info requested
   print ("\n], \"perPage\" : " .. perPage .. ",\n")
end

if(sortColumn == nil) then
   sortColumn = ""
end

if(sortOrder == nil) then
   sortOrder = ""
end

if (as_n == nil) then -- single AS info requested
   print ("\"sort\" : [ [ \"" .. sortColumn .. "\", \"" .. sortOrder .."\" ] ],\n")
   print ("\"totalRows\" : " .. total .. " \n}")
end
