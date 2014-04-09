--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')
local debug = false

-- Output parameters
mode = _GET["mode"]

-- Table parameters
currentPage = _GET["currentPage"]
perPage     = _GET["perPage"]
sortColumn  = _GET["sortColumn"]
sortOrder   = _GET["sortOrder"]
host        = _GET["host"]
port        = _GET["port"]
application = _GET["application"]

-- Host comparison parameters
key = _GET["key"]

-- System host parameters
hosts = _GET["hosts"]
user = _GET["user"]
pid = tonumber(_GET["pid"])
name = _GET["name"]


if(mode == nil) then
   mode = "table"
end
if(debug) then io.write("Mode: "..mode.."\n") end

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

if(port ~= nil) then port = tonumber(port) end

if(sortOrder == nil) then
   sortOrder = "asc"
end

interface.find(ifname)
flows_stats = interface.getFlowsInfo()


if (mode == "table") then
  print ("{ \"currentPage\" : " .. currentPage .. ",\n \"data\" : [\n")
  to_skip = (currentPage-1) * perPage
end


total = 0
processes = {}
vals = {}
num = 0

for _key, value in pairs(flows_stats) do
  p = flows_stats[_key]
  process = 1 
  
  ---------------- PID ----------------
   if(pid ~= nil) then
    if (debug) then io.write("Pid:"..pid.."\n")end
    if (p["client_process"] ~= nil) then 
      if (debug) then io.write("Client pid:"..p["client_process"]["pid"].."\n") end
      if ((p["client_process"]["pid"] ~= pid)) then 
        process = 0
      end
      if (debug) then io.write("ClientProcess -\t"..process.."\n")end
    end
    if (p["server_process"] ~= nil) then 
      if (debug) then io.write("Server pid:"..p["server_process"]["pid"].."\n") end
      if ((p["server_process"]["pid"] ~= pid)) then 
        process = 0
      end
      if (debug) then io.write("ServerProcess -\t"..process.."\n")end
    end
   end
   if (debug) then io.write("Pid -\t"..process.."\n")end

  ---------------- NAME ----------------
   if(name ~= nil) then
    if (debug) then io.write("Name:"..name.."\n")end
    if (p["client_process"] ~= nil) then 
      if (debug) then io.write("Client name:"..p["client_process"]["name"].."\n") end
      if ((p["client_process"]["name"] ~= name)) then 
        process = 0
      end
      if (debug) then io.write("ClientProcess -\t"..process.."\n")end
    end
    if (p["server_process"] ~= nil) then 
      if (debug) then io.write("Server name:"..p["server_process"]["name"].."\n") end
      if ((p["server_process"]["name"] ~= name)) then 
        process = 0
      end
      if (debug) then io.write("ServerProcess -\t"..process.."\n")end
    end
   end
   if (debug) then io.write("name -\t"..process.."\n")end

  ---------------- HOST ----------------
  if((host ~= nil) and (p["cli.ip"] ~= host) and (p["srv.ip"] ~= host)) then
    process = 0
  end


  if (process == 1) then

    if(p["client_process"] ~= nil) then 
      k = p["client_process"]
      key = k["name"]

      if(processes[key] == nil) then
    processes[key] = { }
    -- Flow information
    processes[key]["bytes_sent"] = p["cli2srv.bytes"]
    processes[key]["bytes_rcvd"] = p["srv2cli.bytes"]
    processes[key]["duration"] = p["duration"]
    processes[key]["count"] = 1
    -- Process information
    processes[key]["actual_memory"] = p["client_process"]["actual_memory"]
    processes[key]["average_cpu_load"] = p["client_process"]["average_cpu_load"]
      else
    -- Flow information
    processes[key]["duration"] = math.max(processes[key]["duration"], p["duration"])
    processes[key]["bytes_sent"] = processes[key]["bytes_sent"] + p["cli2srv.bytes"]
    processes[key]["bytes_rcvd"] = processes[key]["bytes_rcvd"] + p["srv2cli.bytes"]
    processes[key]["count"] = processes[key]["count"] + 1
    -- Process information
    processes[key]["actual_memory"] = processes[key]["actual_memory"] + p["client_process"]["actual_memory"]
    processes[key]["average_cpu_load"] = processes[key]["average_cpu_load"] + p["client_process"]["average_cpu_load"]
      end
    end

    if(p["server_process"] ~= nil) then 
      k = p["server_process"]
      key = k["name"]

      if(processes[key] == nil) then
    processes[key] = { }
    -- Flow information
    processes[key]["bytes_sent"] = p["srv2cli.bytes"]
    processes[key]["bytes_rcvd"]  = p["cli2srv.bytes"]
    processes[key]["duration"] = p["duration"]
    processes[key]["count"] = 1
    -- Process information
    processes[key]["actual_memory"] = p["server_process"]["actual_memory"]
    processes[key]["average_cpu_load"] = p["server_process"]["average_cpu_load"]
      else
    -- Flow information
    processes[key]["duration"] = math.max(processes[key]["duration"], p["duration"])
    processes[key]["bytes_sent"] = processes[key]["bytes_sent"] + p["srv2cli.bytes"]
    processes[key]["bytes_rcvd"] = processes[key]["bytes_rcvd"] + p["cli2srv.bytes"]
    processes[key]["count"] = processes[key]["count"] + 1
    -- Process information
    processes[key]["actual_memory"] = processes[key]["actual_memory"] + p["server_process"]["actual_memory"]
    processes[key]["average_cpu_load"] = processes[key]["average_cpu_load"] + p["server_process"]["average_cpu_load"]
      end
    end
  end
end


-- Aggregated value

for key, value in pairs(processes) do
  -- Process information
  processes[key]["actual_memory"] = (processes[key]["actual_memory"] / processes[key]["count"])
  processes[key]["average_cpu_load"] = (processes[key]["average_cpu_load"] / processes[key]["count"])

end

-- Sorting table

for key, value in pairs(processes) do
      -- postfix is used to create a unique key otherwise entries with the same key will disappear
      num = num + 1
      postfix = string.format("0.%04u", num)
      if(sortColumn == "column_name") then
   vkey = key
   elseif(sortColumn == "column_bytes_rcvd") then
   vkey = processes[key]["bytes_rcvd"]+postfix
   elseif(sortColumn == "column_bytes_sent") then
   vkey = processes[key]["bytes_sent"]+postfix
   elseif(sortColumn == "column_duration") then
   vkey = processes[key]["duration"]+postfix    
   elseif(sortColumn == "column_count") then
   vkey = processes[key]["count"]+postfix   
      else
   vkey = key
      end
      
      vals[vkey] = key
end

num = 0
table.sort(vals)

if(sortOrder == "asc") then
   funct = asc
else
   funct = rev
end


-- Json output

if (mode == "table") then
  for _key, _value in pairsByKeys(vals, funct) do
     key = vals[_key]   
     value = processes[key]

     if(to_skip > 0) then
        to_skip = to_skip-1
     else
        if(num < perPage) then
     if(num > 0) then
        print ",\n"
     end
     srv_tooltip = ""
     cli_tooltip = ""

    
     print ("{ \"column_name\" : \"".."<A HREF='/lua/get_process_info.lua?name=" .. key .. "'>".. key .. "</A>")

     print ("\", \"column_duration\" : \"" .. secondsToTime(value["duration"]))
     print ("\", \"column_count\" : \"" .. value["count"])
     print ("\", \"column_bytes_sent\" : \"" .. bytesToSize(value["bytes_sent"]) .. "")
     print ("\", \"column_bytes_rcvd\" : \"" .. bytesToSize(value["bytes_rcvd"]) .. "")

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

elseif (mode == "timeline") then

  print ("[\n")
  for _key, _value in pairsByKeys(vals, funct) do
     key = vals[_key]   
     value = processes[key]

     if (num > 0) then print(',\n') end

    print('{'..
      '\"name\":\"'       .. key                                                .. '\",' ..
      '\"label\":\"'      .. key                                                .. '\",' ..
      '\"value\":'        .. (value["bytes_sent"] + value["bytes_rcvd"])        .. ',' ..
      '\"memory\":'       .. value["actual_memory"]                             .. ',' ..
      '\"cpu\":'          .. value["average_cpu_load"]                          ..
    '}')
    num = num + 1

  end
  print ("]")

end
