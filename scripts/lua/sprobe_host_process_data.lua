--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('application/json')

host_ip = _GET["host"]
host_id = _GET["host_id"]

interface.find(ifname)
flows_stats = interface.getFlowsInfo(host_ip)

procs = {}
links = {}

hosts = {}

for key, value in pairs(flows_stats) do
   flow = flows_stats[key]

   if((flow["client_process"] ~= nil)
      and (flow["server_process"] ~= nil)) then
      c = flow["client_process"]["pid"]
      s = flow["server_process"]["pid"]

      procs[c] = flow["client_process"]
      procs[s] = flow["server_process"]

      links[c] = s
      links[s] = c
      elseif(flow["client_process"] ~= nil) then
      hosts[flow["client_process"]["pid"]] = { flow["srv.ip"], flow["srv.ip"] }
      procs[flow["client_process"]["pid"]] = flow["client_process"]
      elseif(flow["server_process"] ~= nil) then
      hosts[flow["server_process"]["pid"]] = { flow["cli.ip"], flow["srv.ip"] }
      procs[flow["server_process"]["pid"]] = flow["server_process"]
   end
end

print("[")
n = 0
for key, value in pairs(links) do
   if(n > 0) then print(",") end
   print('\n\t{"source": "'..key..'", "source_type": "proc", "source_pid": '.. procs[key]["pid"] ..', "source_name": "'.. procs[key]["name"] ..'", "target": "'..value..'", "target_type": "proc", "target_pid": '.. procs[value]["pid"] ..', "target_name": "'.. procs[value]["name"] ..'", "type": "proc2proc"}')
   n = n + 1
end

for key, value in pairs(hosts) do
   if(n > 0) then print(",") end

   if(procs[key]["name"] ~= nil) then
      print('\n\t{"source": "'..key..'", "source_type": "proc", "source_pid": '.. procs[key]["pid"] ..', "source_name": "'.. procs[key]["name"].."@".. value[2] ..'", "target": "'..value[1]..'", "target_type": "host", "target_pid": -1, "target_name": "'.. ntop.getResolvedAddress(value[1]) ..'", "type": "proc2host"}')
   else
      print('\n\t{"source": "'..value..'", "source_type": "host", "source_pid": -1, "source_name": "'.. ntop.getResolvedAddress(value) ..'", "target": "'..key'", "target_type": "proc", "target_pid": '.. procs[key]["pid"] ..', "target_name": "'.. procs[key]["name"] ..'", "type": "host2proc"}')
   end
   n = n + 1
end
print("\n]\n")

