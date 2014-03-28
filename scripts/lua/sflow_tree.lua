
--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')

function setAggregatedFlow(pid,father_pid,father_name,p_what,p_how,type)
 if (aggregated_flows[pid] == nil) then
    aggregated_flows[pid] = {};
    aggregated_flows[pid]["pid"] = pid
    aggregated_flows[pid]["father_pid"] = father_pid
    aggregated_flows[pid]["father_name"] = father_name
    aggregated_flows[pid]["type"] = type
    aggregated_flows[pid][what] = p_what
    aggregated_flows[pid][how] = p_how 
  else
    -- Aggregate values
    aggregated_flows[pid][how] = aggregated_flows[pid][how] + p_how 
  end
end

mode = _GET["mode"] -- memory(actual-memory),bytes
type = _GET["type"] -- user,process(proc_name)
host = _GET["host"]

interface.find(ifname)
local debug = false

flows_stats = interface.getFlowsInfo()


aggregated_flows = {}
father_process = {}
cildren_proess = {}
num = 0


-- Process parameter
  if((type == nil) or (type == "memory")) then
    how = "actual_memory"
    how_is_process = 1
  elseif (type == "bytes") then
    how = "bytes"
  end

  if((mode == nil) or (mode == "process")) then
    what = "name"
    url = "/lua/get_process_info.lua?pid="
  end


for key, value in pairs(flows_stats) do
  flow = flows_stats[key]
  process = 1

  if ((flow["cli.ip"] ~= host) and 
    (flow["srv.ip"] ~= host)) then 
    process = 0
  end


  if (process == 1) then

    if (flow["cli.ip"] == host) and (flow["client_process"] ~= nil) then 
      client_id = flow["client_process"]["pid"]
      if (how_is_process == 1) then
        client_how = flow["client_process"][how]
      else
        client_how = flow["cli2srv.bytes"]
      end
      setAggregatedFlow(client_id,flow["client_process"]["father_pid"],flow["client_process"]["father_name"],flow["client_process"][what],client_how,"client")
    end
      
    if (flow["srv.ip"] == host) and (flow["server_process"] ~= nil) then 
      server_id = flow["server_process"]["pid"]
      if (how_is_process == 1) then
        server_how = flow["server_process"][how]
      else
        server_how = flow["srv2cli.bytes"]
      end
      setAggregatedFlow(server_id,flow["server_process"]["father_pid"],flow["server_process"]["father_name"],flow["server_process"][what],server_how,"server")
    end

  end
end 


father_process = {}
cildren_proess = {}
num = 0
tot = 0
for key, value in pairs(aggregated_flows) do
  flow = aggregated_flows[key]
  -- print("Pid:"..flow["pid"]..", Father Pid:"..flow["father_pid"]..", Father Name:"..flow["father_name"]..", Type:"..flow["type"]..", What:"..flow[what]..", How:"..flow[how].."\n")

  if(flow["pid"] == flow["father_pid"]) then
    father_process[flow["pid"]] = {}
    father_process[flow["pid"]]["name"] = flow[what]
    father_process[flow["pid"]]["size"] = flow[how]
     tot = tot + father_process[flow["pid"]]["size"]
  else
    if(father_process[flow["father_pid"]] == nil) then
      father_process[flow["father_pid"]] = {}
      father_process[flow["father_pid"]]["children"] = {}
      father_process[flow["father_pid"]]["children"][flow["pid"]] = {}
      father_process[flow["father_pid"]]["children"][flow["pid"]]["name"] = flow[what]
      father_process[flow["father_pid"]]["children"][flow["pid"]]["size"] = flow[how]
      father_process[flow["father_pid"]]["size"] = flow[how]
      father_process[flow["father_pid"]]["name"] = flow["father_name"]
    else
      father_process[flow["father_pid"]]["children"][flow["pid"]] = {}
      father_process[flow["father_pid"]]["children"][flow["pid"]]["name"] = flow[what]
      father_process[flow["father_pid"]]["children"][flow["pid"]]["size"] = flow[how]
      father_process[flow["father_pid"]]["size"] = father_process[flow["father_pid"]]["size"] + flow[how]
  end
  tot = tot + father_process[flow["father_pid"]]["size"]

end

end

-- for key, size in pairs(father_process) do
--   flow = father_process[key]
--   print("Father => Name:"..flow["name"]..", size:"..flow["size"].."\n")
--   if (flow["children"] ~= nil) then
--     for key, size in pairs(flow["children"]) do
--       children = flow["children"][key]
--       print("Children => Name:"..children["name"]..", size:"..children["size"].."\n")
--     end
--   end
-- end



print "{\n"
num = 0
s = 0
print("\"name\": \"" .. what .."\", \"size\": ".. tot ..", \n\"children\": [\n") 

for key, value in pairs(father_process) do
  flow = father_process[key]
  if(num > 0) then
   print ",\n"
  end

  print("\t { \"name\": \"" .. flow["name"] .."\", \"id\": " .. key ..", \"size\": ".. flow["size"]..",\"url\": \"" .. url..key.."\"") 
  
  
  children_num = 0
  if (flow["children"] ~= nil) then
    print( ", \n\t\"children\": \n\t\t[\n")
    for k,v in pairs(flow["children"]) do
      children = flow["children"][k]
      if(children_num > 0) then
        print ",\n"
      end
    
      print("\t\t { \"name\": \"" .. children["name"] .."\", \"id\": ".. k ..", \"size\": ".. children["size"] ..",\"url\": \"" .. url..k.."\" }") 
    
      children_num = children_num + 1
    
    end
    print ("\n\t\t]\n")
  end

  print ("\t}")
  
 num = num + 1

end


print "]\n}"

