--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"

sendHTTPHeader('text/html; charset=iso-8859-1')
-- sendHTTPHeader('application/json')
interface.find(ifname)

aggregated  = _GET["aggregated"]
host_info = url2hostinfo(_GET)

interface.find(ifname)

if(host_info["host"] ~= nil) then
  if(aggregated == nil) then
    host = interface.getHostInfo(host_info["host"],host_info["vlan"]) 
  else
    host = interface.getAggregatedHostInfo(host_info["host"])
  end   
else
 host = nil
end


if(host == nil) then
 print('{}')
else

 print('{')
 now = os.time()
 -- Get from redis the throughput type bps or pps
 throughput_type = getThroughputType()

 print("\"column_since\" : \"" .. secondsToTime(now-host["seen.first"]+1) .. "\", ")
 print("\"column_last\" : \"" .. secondsToTime(now-host["seen.last"]+1) .. "\", ")
 
 if (aggregated == nil) then

  print("\"column_traffic\" : \"" .. bytesToSize(host["bytes.sent"]+host["bytes.rcvd"]).. "\", ")

  if (  (host["throughput_trend_"..throughput_type] ~= nil) and 
        (host["throughput_trend_"..throughput_type] > 0) 
  ) then

    if (throughput_type == "pps") then
      print ("\"column_thpt\" : \"" .. pktsToSize(host["throughput_bps"]).. " ")
    else
      print ("\"column_thpt\" : \"" .. bitsToSize(8*host["throughput_bps"]).. " ")
    end

    if(host["throughput_trend_"..throughput_type] == 1) then 
      print("<i class='fa fa-arrow-up'></i>")
    elseif(host["throughput_trend_"..throughput_type] == 2) then
      print("<i class='fa fa-arrow-down'></i>")
    elseif(host["throughput_trend_"..throughput_type] == 3) then
      print("<i class='fa fa-minus'></i>")
    end
    print("\",")
  else
    print ("\"column_thpt\" : \"0 "..throughput_type.."\",")
  end

  sent2rcvd = round((host["bytes.sent"] * 100) / (host["bytes.sent"]+host["bytes.rcvd"]), 0)
  print ("\"column_breakdown\" : \"<div class='progress'><div class='progress-bar progress-bar-warning' style='width: "
  .. sent2rcvd .."%;'>Sent</div><div class='progress-bar progress-bar-info' style='width: " .. (100-sent2rcvd) .. "%;'>Rcvd</div></div>")
  
  else
  -- Aggregated


    if(host["throughput_trend_bps"] > 0) then 
      print("\"column_queries\" : \"" .. formatValue(host["queries.rcvd"]).." ")

      if(host["throughput_trend_bps"] == 1) then
        print("<i class='fa fa-arrow-up fa-lg'></i>")
      elseif(host["throughput_trend_bps"] == 2) then
        print("<i class='fa fa-arrow-down fa-lg'></i>")
      elseif(host["throughput_trend_bps"] == 3) then
        print("<i class='fa fa-minus fa-lg'></i>")
      end
      
    else
      print ("\"column_queries\" : \"NaN")
    end

  end
  
  print("\" } ")

end
