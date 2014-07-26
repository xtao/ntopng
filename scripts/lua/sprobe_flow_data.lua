--
-- (C) 2014 - ntop.org
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

if(flow == nil) then
   print("{ } ")
else
   key = "" -- TODO

   -- ====================================

   function nest2tab(level)
      print('\n')

      while(level > 0) do
	 print('\t')
	 level = level - 1
      end
   end

   -- ====================================

   function displayProc(nest, proc, host, host_name,
			add_host, add_father, first_element, last_element)
      -- if(num > 0) then print(',') end

      if(add_host) then
	 nest2tab(nest)
	 link = "/lua/host_details.lua?host=".. host .."&page=flows"
	 print('{ "name": "'..host_name..'", "type": "host", "link": "'..link..'", "children": [ ')
	 nest = nest + 1
      end
      
      if(add_father) then
	 if(first_element and (proc.father_pid ~= 1)) then
	    nest2tab(nest)
	    print('{ "name": "init (pid 1)", "type": "proc", "children": [ ')
	    nest = nest + 1
	 else
	    if(not(first_element)) then
	       nest2tab(nest)
	       print('] },')
	       nest = nest -1
	    end
	 end
	 
	 -- No link for father
	 -- link = "/lua/get_process_info.lua?pid="..proc.father_pid.."&name="..proc.father_name.."&host=".. host .."&page=flows"
	 nest2tab(nest)
	 print('{ "name": "'..proc.father_name..' (pid '.. proc.father_pid..')", "type": "proc", "children": [ ')
	 nest = nest + 1
      end

      link = "/lua/get_process_info.lua?pid="..proc.pid.."&name="..proc.name.."&host=".. host .."&page=Flows"
      nest2tab(nest)
      print('{ "name": "'..proc.name..' (pid '.. proc.pid..')", "link": "'.. link ..'", "type": "proc", "children": [ ] }')

      if(last_element) then
	 while(nest > 0) do
	    nest2tab(nest)
	    print('] }')
	    nest = nest -1
	 end
      end

      return(nest)
   end

   nest = 0

   if((flow.client_process ~= nil) and (flow.server_process ~= nil)) then
      if(flow["cli.ip"] ~= flow["srv.ip"]) then 
	 print('{ "name": "", "type": "root", "children": [') 
	 last = true
      else
	 last = false
      end
      -- nest = nest + 1
      nest = displayProc(nest, flow.client_process, 
			 flow["cli.ip"], 
			 flowinfo2hostname(flow, "cli", flow["vlan"]),
			 true, true, true, last)

      print(",\n")

      displayProc(nest, flow.server_process, 
		  flow["srv.ip"], 
		  flowinfo2hostname(flow, "srv", flow["vlan"]),
		  (flow["cli.ip"] ~= flow["srv.ip"]), 
		  ((flow.client_process.father_pid ~= flow.server_process.father_pid) or (flow["cli.ip"] ~= flow["srv.ip"])),
		  (flow["cli.ip"] ~= flow["srv.ip"]), true)
      if(flow["cli.ip"] ~= flow["srv.ip"]) then
	 print("] }\n")
      end
   elseif(flow.client_process ~= nil) then
      nest = displayProc(nest, flow.client_process, 
			 flow["cli.ip"], flowinfo2hostname(flow, "cli", flow["vlan"]),
			 true, true, true, true)
   elseif(flow.server_process ~= nil) then
      nest = displayProc(nest, flow.server_process,
			 flow["srv.ip"], flowinfo2hostname(flow, "srv", flow["vlan"]),
			 true, true, true, true)
   end

   if(false) then 
      if(flow["cli.ip"] == flow["srv.ip"]) then
	 print [[
	    {
	       "name": "init (pid 1)",
	       "type": "proc",
	       "children": [ ]]
      else
	 print('{  "name": "", "type": "root", "children": [\n')
      end
      
      print(']\n}')
   end
end