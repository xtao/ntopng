--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.workingdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')

ifname      = _GET["if"]
currentPage = _GET["currentPage"]
perPage     = _GET["perPage"]
sortColumn  = _GET["sortColumn"]
sortOrder   = _GET["sortOrder"]
host        = _GET["host"]
port        = _GET["port"]

if(currentPage == nil) then
   currentPage = 1
else
   currentPage = tonumber(currentPage)
end

if(perPage == nil) then
   perPage = 10
else
   perPage = tonumber(perPage)
end

if(ifname == nil) then	  
  ifname = "any"
end

if(port ~= nil) then port = tonumber(port) end

interface.find(ifname)
flows_stats = interface.getFlowsInfo()

print ("{ \"currentPage\" : " .. currentPage .. ",\n \"data\" : [\n")
total = 0
to_skip = (currentPage-1) * perPage

--host = "a"
vals = {}
num = 0
for key, value in pairs(flows_stats) do
--   print(key.."\n")
   --print("==>"..flows_stats[key]["bytes.sent"].."\n")

   process = 1
   if(host ~= nil) then
      if((flows_stats[key]["src.ip"] ~= host) and (flows_stats[key]["dst.ip"] ~= host)) then
	 process = 0
      end
   end	
   if(port ~= nil) then
      if((flows_stats[key]["src.port"] ~= port) and (flows_stats[key]["dst.port"] ~= port)) then
	 process = 0
      end
   end

   if(process == 1) then 
      -- postfix is used to create a unique key otherwise entries with the same key will disappear
      num = num + 1
      postfix = string.format("0.%04u", num)
      if(sortColumn == "column_client") then
	 vkey = flows_stats[key]["src.ip"]..postfix
	 elseif(sortColumn == "column_server") then
	 vkey = flows_stats[key]["dst.ip"]..postfix
	 elseif(sortColumn == "column_bytes") then
	 vkey = flows_stats[key]["bytes"]+postfix
	 elseif(sortColumn == "column_ndpi") then
	 vkey = flows_stats[key]["proto.ndpi"]..postfix
	 elseif(sortColumn == "column_category") then
   	 vkey = flows_stats[key]["category"]..postfix
	 elseif(sortColumn == "column_duration") then
	 vkey = flows_stats[key]["duration"]+postfix	  
	 elseif(sortColumn == "column_proto_l4") then
	 vkey = flows_stats[key]["proto.l4"]..postfix
      else
	 -- By default sort by bytes
	 vkey = flows_stats[key]["bytes"]+postfix
      end
      
      --      print("-->"..num.."="..vkey.."\n")
      vals[vkey] = key
      end
end

num = 0
table.sort(vals)

if(sortOrder == "asc") then
   funct = asc
else
   funct = rev
end


for _key, _value in pairsByKeys(vals, funct) do
   key = vals[_key]   
   value = flows_stats[key]

--   print(key.."="..flows_stats[key]["duration"].."\n");
--   print(key.."=".."\n");
    -- print(key.."/num="..num.."/perPage="..perPage.."/toSkip="..to_skip.."\n")	 
   if(to_skip > 0) then
      to_skip = to_skip-1
   else
      if(num < perPage) then
	 if(num > 0) then
	    print ",\n"
	 end

	 name = value["src.host"]
	 if(name == "") then
	    name = value["src.ip"]
	 end

	 src_key="<A HREF='/lua/host_details.lua?interface=".. ifname .. "&host=" .. value["src.ip"] .. "'>".. abbreviateString(name, 20) .."</A>"
	 if(value["src.port"] > 0) then
  	   src_port=":<A HREF='/lua/port_details.lua?interface=".. ifname .. "&port=" .. value["src.port"] .. "'>"..value["src.port"].."</A>"
         else
	   src_port=""
         end

	 name = value["dst.host"]
	 if(name == "") then
	    name = value["dst.ip"]
	 end

	 dst_key="<A HREF='/lua/host_details.lua?interface=".. ifname .. "&host=" .. value["dst.ip"] .. "'>".. abbreviateString(name, 20) .."</A>"
	 if(value["dst.port"] > 0) then
  	   dst_port=":<A HREF='/lua/port_details.lua?interface=".. ifname .. "&port=" .. value["dst.port"] .. "'>"..value["dst.port"].."</A>"
         else
	   dst_port=""
         end

	 descr=value["src.host"]..":"..value["src.port"].." &lt;-&gt; "..value["dst.host"]..":"..value["dst.port"]
	 print ("{ \"column_key\" : \"<A HREF='/lua/flow_details.lua?interface=".. ifname .. "&flow_key=" .. key .. "&label=" .. descr.."'><span class='label label-info'>Info</span></A>")
	 print ("\", \"column_client\" : \"" .. src_key .. src_port)
	 print ("\", \"column_server\" : \"" .. dst_key .. dst_port)
	 print ("\", \"column_vlan\" : \"" .. value["vlan"])
	 print ("\", \"column_category\" : \"" .. getCategory(value["category"]))
	 -- io.write(value["category"].."[" .. getCategory(value["category"]).. "]\n")	 
	 print ("\", \"column_proto_l4\" : \"" .. value["proto.l4"])
	 print ("\", \"column_ndpi\" : \"" .. value["proto.ndpi"])
	 print ("\", \"column_duration\" : \"" .. secondsToTime(value["duration"]))
	 print ("\", \"column_bytes\" : \"" .. bytesToSize(value["bytes"]) .. "")

	 cli2srv = round((value["cli2srv.bytes"] * 100) / value["bytes"], 0)
	 print ("\", \"column_breakdown\" : \"<div class='progress'><div class='bar bar-warning' style='width: " .. cli2srv .."%;'>Client</div><div class='bar bar-info' style='width: " .. (100-cli2srv) .. "%;'>Server</div></div>")

	 print ("\" }\n")
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
