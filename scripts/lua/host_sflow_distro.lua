--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "flow_utils"
local json = require ("dkjson")

sendHTTPHeader('text/html')

mode = _GET["mode"] -- user,process(proc_name) 
type = _GET["type"] -- memory(actual-memory),bytes,latency
host = _GET["host"]
filter = _GET["filter"] -- all,client,server

interface.find(ifname)
local debug = false

if(host == nil) then
  print("<div class=\"alert alert-error\"><img src=/img/warning.png> This flow cannot be found (expired ?)</div>")
else

  flows_stats = interface.getFlowsInfo()
  
  -- Default values
  filter_client = 0
  filter_server = 0
  how_is_process = 0
  how_is_latency = 0

  -- Process parameter
  if((type == nil) or (type == "memory")) then
    how = "actual_memory"
    how_is_process = 1
  elseif (type == "bytes") then
    how = "bytes"
  elseif (type == "latency") then
    how_is_latency = 1
    how = "Application latency (residual usec)"
  end
  if (debug) then io.write("How:"..how.."\n"); end
  if((mode == nil) or (mode == "user")) then
    what = "user_name"
    url = "/lua/get_user_info.lua?user="
  elseif (mode == "process") then
    what = "name"
    url = "/lua/get_process_info.lua?name="
  end

  if((filter == nil) or (filter == "All")) then
    filter_client = 1
    filter_server = 1
  elseif (filter == "Client") then
    filter_client = 1
  elseif (filter == "Server") then
    filter_server = 1
  end

  tot = 0
  what_array = {}
  num = 0
  
  for key, value in pairs(flows_stats) do
    client_process = 0
    server_process = 0
    flow = flows_stats[key]
    if (debug) then io.write("Client:"..flow["cli.ip"]..",Server:"..flow["srv.ip"].."\n"); end
    
    if((filter_client == 1) and (flow["cli.ip"] == host) and (flow.client_process ~= nil))then
      client_process = 1
    end

    if((filter_server == 1) and (flow["srv.ip"] == host) and (flow.server_process ~= nil))then
      server_process = 1
    end

    
    if ((client_process == 1))then
      current_what = flow["client_process"][what].." (client)"
      if (how_is_process == 1) then
        value = flow["client_process"][how] 
      elseif (how_is_latency == 1) then
        flow_more_info = interface.findFlowByKey(key)
        local info, pos, err = json.decode(flow_more_info["moreinfo.json"], 1, nil)
        for k,v in pairs(info) do
          if("Application latency (residual usec)" == getFlowKey(k)) then
            value = tonumber(handleCustomFlowField(k, v))
            io.write("Client val:"..value.."-"..flow["client_process"][what]..'\n')
          end
        end
      else
        value = flow["cli2srv.bytes"]
      end
      
      if (what_array[current_what] == nil) then 
        what_array[current_what]  = {}
        what_array[current_what]["value"]  = 0
        what_array[current_what]["url"]  = url..flow["client_process"][what].."&host="..flow["cli.ip"]
      end
      
       if ((how_is_process == 1) or (how_is_latency == 1))then
        if ( what_array[current_what]["value"]  == 0) then
          what_array[current_what]["value"] = value
        end
      else
        what_array[current_what]["value"] = what_array[current_what]["value"] + value
      end

      if (debug) then io.write("Find client_process:"..current_what..", Value:"..value..", Process:"..flow["client_process"]["name"]..",Pid:"..flow["client_process"]["pid"]..",Url:"..what_array[current_what]["url"].."\n"); end

    end
    
    if(server_process == 1) then
      current_what = flow["server_process"][what].." (server)"
      
      if (how_is_process == 1) then
       
       value = flow["server_process"][how] 
      
      elseif (how_is_latency == 1) then
      
        flow_more_info = interface.findFlowByKey(key)
      
        local info, pos, err = json.decode(flow_more_info["moreinfo.json"], 1, nil)
        for k,v in pairs(info) do
          if("Application latency (residual usec)" == getFlowKey(k)) then
            value = tonumber(handleCustomFlowField(k, v))
            io.write("Server val:"..value.."-"..flow["server_process"][what]..'\n')
          end
        end
      
      else
      
        value = flow["srv2cli.bytes"]
      
      end
      
      if (what_array[current_what] == nil) then 
      
        what_array[current_what]  = {}
        what_array[current_what]["value"]  = 0
        what_array[current_what]["url"]  = url..flow["server_process"][what].."&host="..flow["srv.ip"]
      
      end
      
      if ((how_is_process == 1) or (how_is_latency == 1))then
        if ( what_array[current_what]["value"]  == 0) then
          what_array[current_what]["value"] = value
        end
      else
        what_array[current_what]["value"] = what_array[current_what]["value"] + value
      end
      
      tot = tot + value
      
      if (debug) then io.write("Find server_process:"..current_what..", Value:"..value..", Process:"..flow["server_process"]["name"]..",Pid:"..flow["server_process"]["pid"]..",Url:"..what_array[current_what]["url"].."\n"); end

    end
  end
  
  -- Print json
  print "[\n"
  num = 0
  s = 0

  tot = 0
  for key, value in pairs(what_array) do
     value = what_array[key]["value"]
     tot = tot + value
  end

  other = 0;
  thr = (tot * 5) / 100
  
  for key, value in pairs(what_array) do
     value = what_array[key]["value"]
     -- io.write("Val: "..value.."\n")
     if(value >= thr) then
	if(num > 0) then
	   print ",\n"
	end
	label = key
	url = what_array[key]["url"]
	print("\t { \"label\": \"" .. label .."\", \"value\": ".. value ..", \"url\": \"" .. url.."\" }") 
	num = num + 1
	s = s + value
     end
  end

  if(tot > s) then
    print(",\t { \"label\": \"Other\", \"value\": ".. (tot-s) .." }") 
  end

  print "\n]"

end