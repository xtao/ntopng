--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('application/json')

interface.find(ifname)
flows_stats = interface.getFlowsInfo()

procs = {}
hosts = {}
names = {}

for key, value in pairs(flows_stats) do
   flow = flows_stats[key]

   c = flow["cli.ip"]
   s = flow["srv.ip"]

   if(flow["cli.host"] ~= nil) then c_sym = flow["cli.host"] else c_sym = ntop.getResolvedAddress(flow["cli.ip"]) end
   if(flow["srv.host"] ~= nil) then s_sym = flow["srv.host"] else s_sym = ntop.getResolvedAddress(flow["srv.ip"]) end

   names[c] = c_sym
   names[s] = s_sym
   --c = c .. "@" .. flow["cli.source_id"]
   --s = s .. "@" .. flow["srv.source_id"]

   if(flow["client_process"] ~= nil) then
      if(hosts[c] == nil) then hosts[c] = { } end
      name = flow["client_process"]["name"]
      if(hosts[c][name] == nil) then hosts[c][name] = { flow["client_process"], { } } end
      hosts[c][name][2][s] = 1
   end

   if(flow["server_process"] ~= nil) then
      if(hosts[s] == nil) then hosts[s] = { } end      
      name = flow["server_process"]["name"]
      if(hosts[s][name] == nil) then hosts[s][name] = { flow["server_process"], { } } end
      hosts[s][name][2][c] = 1

   end
end


n = 0

print [[
{
 "name": "root",
 "children": [ ]]

for key, value in pairs(hosts) do
   if(n > 0) then print(",") end

    print('\n\t{ "name": "'..names[key]..'", "children": [')

    m = 0
    for k, _v in pairs(value) do
       if(m > 0) then print(",") end
       m = m + 1
       v = _v[1]
       -- Process
       link = "/lua/get_process_info.lua?pid="..v["pid"].."&name="..k.."&host=".. key .."&page=Flows"
       print('\n\t\t{ "name": "'..k..' (pid '.. v["pid"]..')", "link": "'.. link ..'", "children": [ ')
       o = 0
       for peer,_ in pairs(_v[2]) do
	  if(peer ~= key) then
	     if(o > 0) then print(",") end
	     o = o + 1
	     link = "/lua/host_details.lua?host="..peer .."&page=flows"
	     print('\n\t\t\t{ "name": "'..names[peer]..'", "link": "'.. link ..'", "children": [ ] } ')	  
	  end
       end
--       print('\n\t\t\t] }')
       print('\n\t\t] }')
    end

    print('\n\t] }')

   n = n + 1
end



print("\n]\n}\n")


