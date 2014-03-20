--
-- (C) 2014 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('application/json')



-- Defaul value
local debug = false
interface.find(ifname)
aggregation = "ndpi"

max_num_hosts = 24

compared_hosts = {}
compared_hosts_size = 0;

if(debug) then io.write("==== hosts_compared_sankey ====\n") end
hosts = _GET["hosts"]
if(debug) then io.write("Host:"..hosts.."\n") end

if (_GET["hosts"] ~= nil) then

  compared_hosts, compared_hosts_size = getHostCommaSeparatedList(_GET["hosts"])

  if (compared_hosts_size >= 2) then

    if(_GET["aggregation"] ~= nil) then
        aggregation = _GET["aggregation"]
    end

    -- 1.    Find all flows between compared hosts
    flows_stats = interface.getFlowsInfo()
    
    ndpi = {}
    l4 = {}
    ports = {}

    aggregation_value = {}
    aggregation_value_size = 1
    num = 0
    for key, value in pairs(flows_stats) do

        process = 0
        if ((findStringArray(flows_stats[key]["cli.ip"],compared_hosts) ~= nil) and
            (findStringArray(flows_stats[key]["srv.ip"],compared_hosts) ~= nil))then
        process  = 1
        end -- findStringArray
        
        if (num > max_num_hosts)then process = 0 end

        if (process == 1) then

            if (debug) then io.write("PROCESS => Cli:"..flows_stats[key]["cli.ip"]..",Srv:"..flows_stats[key]["srv.ip"]..",Ndpi:"..flows_stats[key]["proto.ndpi"]..",L4:"..flows_stats[key]["proto.l4"]..",Bytes:"..flows_stats[key]["bytes"].."\n") end
            
            -- 1.1   Save ndpi protocol
            if (aggregation == "ndpi") then

                if (ndpi[flows_stats[key]["proto.ndpi"]] == nil) then
                    aggregation_value[aggregation_value_size] = flows_stats[key]["proto.ndpi"];
                    aggregation_value_size = aggregation_value_size + 1
                    ndpi[flows_stats[key]["proto.ndpi"]] = {}
                    ndpi[flows_stats[key]["proto.ndpi"]]["flows.bytes"] = flows_stats[key]["bytes"]
                else
                    ndpi[flows_stats[key]["proto.ndpi"]]["flows.bytes"] = ndpi[flows_stats[key]["proto.ndpi"]]["flows.bytes"] + flows_stats[key]["bytes"]
                end

                if(debug) then io.write("Ndpi bytes: "..ndpi[flows_stats[key]["proto.ndpi"]]["flows.bytes"].."\n") end
            end

            -- 1.2   Save l4 protocol
            if (aggregation == "l4proto") then

                if (l4[flows_stats[key]["proto.l4"]] == nil) then
                    aggregation_value[aggregation_value_size] = flows_stats[key]["proto.l4"];
                    aggregation_value_size = aggregation_value_size + 1
                    l4[flows_stats[key]["proto.l4"]] = {}
                    l4[flows_stats[key]["proto.l4"]]["flows.bytes"] = flows_stats[key]["bytes"]

                else
                    l4[flows_stats[key]["proto.l4"]]["flows.bytes"] = l4[flows_stats[key]["proto.l4"]]["flows.bytes"] + flows_stats[key]["bytes"]
                end


                if(debug) then io.write("Proto L4 bytes: "..l4[flows_stats[key]["proto.l4"]]["flows.bytes"].."\n") end
            end

            -- 1.3   Save port
            if (aggregation == "port") then
                if(debug) then io.write("Cli port: "..flows_stats[key]["cli.port"].."\n") end
                if(debug) then io.write("Srv port: "..flows_stats[key]["srv.port"].."\n") end
                
                if (ports[flows_stats[key]["cli.port"]] == nil) then
                    aggregation_value[aggregation_value_size] = flows_stats[key]["cli.port"];
                    aggregation_value_size = aggregation_value_size + 1
                    ports[flows_stats[key]["cli.port"]] = {}
                    ports[flows_stats[key]["cli.port"]]["flows.bytes"] = flows_stats[key]["cli2srv.bytes"]
                else
                    ports[flows_stats[key]["cli.port"]]["flows.bytes"] = ports[flows_stats[key]["cli.port"]]["flows.bytes"] + flows_stats[key]["cli2srv.bytes"]
                end


                if (ports[flows_stats[key]["srv.port"]] == nil) then
                    aggregation_value[aggregation_value_size] = flows_stats[key]["srv.port"];
                    aggregation_value_size = aggregation_value_size + 1
                    ports[flows_stats[key]["srv.port"]] = {}
                    ports[flows_stats[key]["srv.port"]]["flows.bytes"] = flows_stats[key]["srv2cli.bytes"]
                else
                    if(debug) then io.write("Srv port: "..flows_stats[key]["srv.port"].."\n") end
                    if(debug) then io.write("Srv bytes: "..ports[flows_stats[key]["srv.port"]]["flows.bytes"].."\n") end

                    ports[flows_stats[key]["srv.port"]]["flows.bytes"] = ports[flows_stats[key]["srv.port"]]["flows.bytes"] + flows_stats[key]["srv2cli.bytes"]

                end
                
            end



            num = num + 1
        end
    end

    print( "{\n\"name\": \"flare\",\n\"children\": [\n")
    num = 0
    for key, value in pairs(aggregation_value) do

     if(num > 0) then
       print ",\n"
   end

   flow_bytes = 1;
   if (aggregation == "port") then
     flow_bytes = ports[aggregation_value[key]]["flows.bytes"]
     elseif (aggregation == "l4proto") then
         flow_bytes = l4[aggregation_value[key]]["flows.bytes"]
     else
         flow_bytes = ndpi[aggregation_value[key]]["flows.bytes"]
     end

     print ("\t{\n\t\"name\": \"" ..aggregation_value[key].. "\",\n\t\"children\": [ \n\t{\"name\": \"" .. aggregation_value[key] .. "\", \"size\": " .. flow_bytes ..", \"aggregation\": \"" .. aggregation .. "\", \"key\": \"" .. aggregation_value[key] .."\"}\n\t]\n\t}")

     num = num + 1

 end

  end --End if (compared host size)
  print ("\n]}\n")
end -- End if _GET[hosts]


-- {
--   "name": "flare",
--   "children": [
--       {
--        "name": "Juniper Networks",
--        "children": [
--         {"name": "Juniper Networks", "size": 159990}
--        ]
--       },
--       {
--        "name": "LinkedIn",
--        "children": [
--         {"name": "LinkedIn", "size": 136427}
--        ]
--       },
--       {
--        "name": "YAHOO!",
--        "children": [
--         {"name": "YAHOO!", "size": 130312}
--        ]
--       }
--     ]
-- }




