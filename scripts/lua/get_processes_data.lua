--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')
local debug = false

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

print ("{ \"currentPage\" : " .. currentPage .. ",\n \"data\" : [\n")
total = 0
to_skip = (currentPage-1) * perPage

processes = {}
vals = {}
num = 0
for _key, value in pairs(flows_stats) do
   p = flows_stats[_key]
   if(p["client_process"] ~= nil) then 
      k = p["client_process"]
      key = k["name"]

      if(processes[key] == nil) then
	 processes[key] = { }
	 processes[key]["bytes_sent"] = p["cli2srv.bytes"]
	 processes[key]["bytes_rcvd"] = p["srv2cli.bytes"]
	 processes[key]["duration"] = p["duration"]
      else
	 processes[key]["duration"] = math.max(processes[key]["duration"], p["duration"])
	 processes[key]["bytes_sent"] = processes[key]["bytes_sent"] + p["cli2srv.bytes"]
         processes[key]["bytes_rcvd"] = processes[key]["bytes_rcvd"] + p["srv2cli.bytes"]
      end
   end

   if(p["server_process"] ~= nil) then 
      k = p["server_process"]
      key = k["name"]

      if(processes[key] == nil) then
	 processes[key] = { }
	 processes[key]["bytes_sent"] = p["srv2cli.bytes"]
	 processes[key]["bytes_rcvd"]  = p["cli2srv.bytes"]
	 processes[key]["duration"] = p["duration"]
      else
	 processes[key]["duration"] = math.max(processes[key]["duration"], p["duration"])
	 processes[key]["bytes_sent"] = processes[key]["bytes_sent"] + p["srv2cli.bytes"]
         processes[key]["bytes_rcvd"] = processes[key]["bytes_rcvd"] + p["cli2srv.bytes"]
      end
   end
end


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
