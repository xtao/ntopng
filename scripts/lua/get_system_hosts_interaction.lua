--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/json')

interface.find(ifname)
flows_stats = interface.getFlowsInfo()


nodes = {}
links = {}
num = 0

-- Create links (flows)

for key, value in pairs(flows_stats) do
  
  -- Find client and server name
  srv_name = flows_stats[key]["srv.host"]
  if((srv_name == "") or (srv_name == nil)) then
    srv_name = flows_stats[key]["srv.ip"]
  end
  srv_name = ntop.getResolvedAddress(srv_name)

  cli_name = flows_stats[key]["cli.host"]
  if((cli_name == "") or (cli_name == nil)) then
    cli_name = flows_stats[key]["cli.ip"]
  end
  cli_name = ntop.getResolvedAddress(cli_name)


  if (flows_stats[key]["client_process"] ~= nil) then 
    client_id = flows_stats[key]["client_process"]["pid"]..'-'..flows_stats[key]["cli.ip"]
    client_name = flows_stats[key]["client_process"]["name"]
  else
    client_id = flows_stats[key]["cli.ip"]
    client_name = abbreviateString(cli_name, 20)
  end
    
  if (flows_stats[key]["server_process"] ~= nil) then 
    server_id = flows_stats[key]["server_process"]["pid"]..'-'..flows_stats[key]["cli.ip"]
    server_name = flows_stats[key]["server_process"]["name"]
  else
    server_id = flows_stats[key]["srv.ip"]
    server_name = abbreviateString(srv_name, 20)
  end

  key_link = client_id.."-"..client_name..":"..server_id.."-"..server_name
  -- print("Key:"..key)
  if (nodes[key_link] == nil) then
    nodes[key_link] = {};
    nodes[key_link]["client_id"] = client_id
    nodes[key_link]["client_name"] = client_name
    nodes[key_link]["server_id"] = server_id
    nodes[key_link]["server_name"] = server_name
    nodes[key_link]["bytes"] = flows_stats[key]["bytes"]
    nodes[key_link]["srv2cli.bytes"] = flows_stats[key]["srv2cli.bytes"]
    nodes[key_link]["cli2srv.bytes"] = flows_stats[key]["cli2srv.bytes"]
  else
    -- Aggregate values
    nodes[key_link]["bytes"] = nodes[key_link]["bytes"] + flows_stats[key]["bytes"]
    nodes[key_link]["cli2srv.bytes"] = nodes[key_link]["cli2srv.bytes"] + flows_stats[key]["cli2srv.bytes"]
    nodes[key_link]["srv2cli.bytes"] = nodes[key_link]["srv2cli.bytes"] + flows_stats[key]["srv2cli.bytes"]
  end
    
end 


print('[\n')

-- Create link (flows)

num = 0
for key, value in pairs(nodes) do

  process = 1

  -- Condition

  -- if ((flows_stats[key]["server_process"] == nil) or 
  --   (flows_stats[key]["client_process"] == nil)) then 
  --   process = 0
  -- end


  -- Get information
  if(process == 1) then
    if (num > 0) then print(',\n') end

    print('{\"client\":\"'..nodes[key]["client_id"]..'\",\"client_name\":\"'..nodes[key]["client_name"]..'\",\"server\":\"'..nodes[key]["server_id"]..'\",\"server_name\":\"'..nodes[key]["server_name"]..'\", \"bytes\":'..nodes[key]["bytes"]..', \"cli2srv_bytes\":'..nodes[key]["cli2srv.bytes"]..', \"srv2cli_bytes\":'..nodes[key]["srv2cli.bytes"]..'}')
    num = num + 1
  end

end

print('\n]')