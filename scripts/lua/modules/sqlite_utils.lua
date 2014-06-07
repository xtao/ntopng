--
-- (C) 2014 - ntop.org
--

-- This file contains the description of all functions
-- used to interact with sqlite

local verbose = false

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "template"
local j = require ("dkjson")

local debug = false

local flow_dir = "flows"

local flow_lua_template = {
  ["IPV4_SRC_ADDR"]   = function (table,val) table["cli.ip"]          = val           end,
  ["L4_SRC_PORT"]     = function (table,val) table["cli.port"]        = tonumber(val) end,
  ["IPV4_DST_ADDR"]   = function (table,val) table["srv.ip"]          = val           end,
  ["L4_DST_PORT"]     = function (table,val) table["srv.port"]        = tonumber(val) end,
  ["PROTOCOL"]        = function (table,val) table["proto.l4"]        = val           end,
  -- ["SRC_VLAN"]        = function (table,val) table["vlan"]            = tonumber(val) end,
  -- ["DST_VLAN"]        = function (table,val) table["vlan"]            = tonumber(val) end,
  ["L7_PROTO_NAME"]   = function (table,val) table["proto.ndpi"]      = val           end,
  ["TCP_FLAGS"]       = function (table,val) table["tcp_flags"]       = tonumber(val) end,
  ["OUT_PKTS"]        = function (table,val) table["cli2srv.packets"] = tonumber(val) end,
  ["OUT_BYTES"]       = function (table,val) table["cli2srv.bytes"]   = tonumber(val) end,
  ["IN_PKTS"]         = function (table,val) table["srv2cli.packets"] = tonumber(val) end,
  ["IN_BYTES"]        = function (table,val) table["srv2cli.bytes"]   = tonumber(val) end,
}

function formatFlows(query)
  flows = {}
  num = 0
 
  rsp = ntop.execQuery(dirs.workingdir ..query , "SELECT * from flows")
  
  if (rsp == nil) then 
    traceError(TRACE_DEBUG,TRACE_CONSOLE,"Query error: ".. query) 
  else

    for _k,_v in pairs(rsp) do

       -- init table of table
      flows[num] = {}
      flows[num]["vlan"] = tonumber(rsp[_k]["vlan_id"])
      flows[num]["bytes"] = tonumber(rsp[_k]["bytes"])
      flows[num]["duration"] = tonumber(rsp[_k]["duration"])
      
      local info, pos, err = j.decode(rsp[_k]["json"], 1, nil)
      
      if (info == nil) then 
        traceError(TRACE_ERROR,TRACE_CONSOLE,"Impossible read json form sqlite: ".. err)
      else

        for key,val in pairs(info) do
          
          label_key = nil

          -- Check if the option --jsonlabes is active
          if (rtemplate[tonumber(key)] ~= nil) then label_key = rtemplate[tonumber(key)] end
          if (template[key] ~= nil) then label_key = key end
          
          -- Convert template id into template name
          if (label_key ~= nil) then 
          
            if (flow_lua_template[label_key] ~= nil) then
              -- Call conversion function in order to convert some value to number if it is necessary
              flow_lua_template[label_key](flows[num],val)
            else  
              -- Leave the default key and value
              flows[num][key] = val
            end

          end
        end -- info loop

        -- Custom parameters
        if (flows[num]["proto.ndpi"] == nil) then
          flows[num]["proto.ndpi"] = "Unknown"
        end
      end -- info == nil

      num = num + 1
    end -- for

  end -- if rsp == nil
  
  if (debug) then
    for key,val in pairs(flows) do
      traceError(TRACE_DEBUG,TRACE_CONSOLE,'Flow: '..key)
      for k,v in pairs(flows[key]) do
        traceError(TRACE_DEBUG,TRACE_CONSOLE,'\t'..k..' - '..v)
      end
    end
  end

  return flows
end
