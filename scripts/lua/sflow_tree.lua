
--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')



mode = _GET["mode"] -- memory(actual-memory),bytes
type = _GET["type"] -- user,process(proc_name)
host = _GET["host"]

interface.find(ifname)
local debug = false

-- if(host == nil) then
 -- print("<div class=\"alert alert-error\"><img src=/img/warning.png> This flow cannot be found (expired ?)</div>")
-- else
  -- Default values
  how_is_process = 0

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

  flow = interface.getFlowsInfo()
  -- Scan flows
  tot = 0
  what_array = {}
  num = 0
  for key, value in pairs(flow) do
    process = 0

    if ((host ~= nil) and (host ~= flow[key]["cli.ip"]) and (host ~= flow[key]["srv.ip"])) then
      process = 1
    end

    if (process == 0) then
      if (what_array[flow[key]["process"]["father_pid"]] == nil) then
        what_array[flow[key]["process"]["father_pid"]] = {}
        what_array[flow[key]["process"]["father_pid"]]["children"] = {}
        what_array[flow[key]["process"]["father_pid"]]["size"] = 0
        what_array[flow[key]["process"]["father_pid"]]["name"] = ""
      end
        children = interface.findFatherPidFlows(flow[key]["process"]["father_pid"])
        
        father_value = 0
        for k, v in pairs(children) do
          
          if (how_is_process == 1) then
            v = children[k]["process"][how] 
          else
            v = children[k][how]
          end
          
          if (what_array[flow[key]["process"]["father_pid"]]["children"][children[k]["process"]["pid"]] == nil) then
            what_array[flow[key]["process"]["father_pid"]]["children"][children[k]["process"]["pid"]] = {}
            what_array[flow[key]["process"]["father_pid"]]["children"][children[k]["process"]["pid"]]["size"] = 0
            what_array[flow[key]["process"]["father_pid"]]["children"][children[k]["process"]["pid"]]["name"] = ""
            old_size = 0
          else
            old_size = what_array[flow[key]["process"]["father_pid"]]["children"][children[k]["process"]["pid"]]["size"];
          end
         
          what_array[flow[key]["process"]["father_pid"]]["children"][children[k]["process"]["pid"]]["size"] = v
          what_array[flow[key]["process"]["father_pid"]]["children"][children[k]["process"]["pid"]]["name"] = children[k]["process"]["name"]
          father_value = father_value + v
          -- io.write("Size:"..what_array[flow[key]["process"]["father_pid"]]["children"][children[k]["process"]["pid"]]["size"].."\n")
          -- io.write("Father:"..father_value.."\n")
          
        end
        
        what_array[flow[key]["process"]["father_pid"]]["size"] = father_value
        what_array[flow[key]["process"]["father_pid"]]["name"] = flow[key]["process"]["father_name"]
        tot = tot + father_value
    end

  end

print "{\n"
num = 0
s = 0
print("\"name\": \"" .. what .."\", \"size\": ".. tot ..", \"children\": [\n") 

for key, value in pairs(what_array) do

  if(num > 0) then
   print ",\n"
  end

  print("\t { \"name\": \"" .. what_array[key]["name"] .."\", \"id\": " .. key ..", \"size\": ".. what_array[key]["size"]..",\"url\": \"" .. url..key.."\"") 
  
  children_num = 0
  print( ", \"children\": [\n")
    
  for k,v in pairs(what_array[key]["children"]) do
    
    if(children_num > 0) then
      print ",\n"
    end
  
    print("\t { \"name\": \"" .. what_array[key]["children"][k]["name"] .."\", \"id\": ".. k ..", \"size\": ".. what_array[key]["children"][k]["size"] ..",\"url\": \"" .. url..k.."\" }") 
  
    children_num = children_num + 1
  
  end

  print ("]\n}")
  
 num = num + 1

end


print "]\n}"

-- end
