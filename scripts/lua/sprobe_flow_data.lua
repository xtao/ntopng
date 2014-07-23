--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "flow_utils"
require "voip_utils"
require "sqlite_utils"

sendHTTPHeader('text/json')

flow_key = _GET["flow_key"]

if(flow_key == nil) then
   flow = nil
else
   interface.find(ifname)
   flow = interface.findFlowByKey(tonumber(flow_key))
end

if((flow.client_process ~= nil) or (flow.server_process ~= nil)) then


same_father = 0
num = 0
key = "" -- TODO

function displayProc(proc, same_host)
   if(num > 0) then print(',') end

   if(not(same_host)) then
print [[
{
 "name": "XXX", 
 "type": "host", 
 "children": [ 
  { "name": "init/1", "type": "proc", "children": [ ]]
end
   
   if((num == 0) or (same_father == 0)) then
      if(proc.father_pid ~= 1) then
	 link = "/lua/get_process_info.lua?pid="..proc.father_pid.."&name="..proc.father_name.."&host=".. key .."&page=Flows"
	 print('\n\t\t{ "name": "'..proc.father_name..' (pid '.. proc.father_pid..')", "link": "'.. link ..'", "type": "proc", "children": [ ')
      end
   end

   if(proc.pid ~= 1) then
      link = "/lua/get_process_info.lua?pid="..proc.pid.."&name="..proc.name.."&host=".. key .."&page=Flows"
      print('\n\t\t{ "name": "'..proc.name..' (pid '.. proc.pid..')", "link": "'.. link ..'", "type": "proc", "children": [ ] }')
   end
  
   
   if(not(same_host)) then 
      print('\t\t\n] } ] } ] ')
      
      if(proc.father_pid == 1) then
	 print('} ]\n')
      end
   else
      if(((num == 0) and (same_father == 0)) or (num > 0)) then
	 if(proc.father_pid ~= 1) then
	    print('\t\t\n] }')
	 end
      end
   end
end

if((flow.client_process ~= nil) and (flow.server_process ~= nil)) then
   if((flow.client_process.father_pid == flow.server_process.father_pid) 
      and (flow["cli.ip"] == flow["srv.ip"])) then
      same_father = 1
   end
end

if(flow["cli.ip"] == flow["srv.ip"]) then
print [[
{
 "name": "init/1", 
 "type": "proc", 
 "children": [ ]]
else
   print('{  "name": "/", "type": "host", "children": [\n')
end

if(flow.client_process ~= nil) then
   displayProc(flow.client_process, (flow["cli.ip"] == flow["srv.ip"]))
   num = num + 1
end

if(flow.server_process ~= nil) then
   displayProc(flow.server_process, (flow["cli.ip"] == flow["srv.ip"]))
   num = num + 1
end

if(flow["cli.ip"] == flow["srv.ip"]) then
   print('\n}')
else
   print('] }\n}\n')
end

end