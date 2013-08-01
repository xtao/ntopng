--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "flow_utils"

ifname = _GET["if"]
if(ifname == nil) then	  
  ifname = "any"
end

flow_key = _GET["flow_key"]
if(flow_key == nil) then
   flow = nil
else
   interface.find(ifname)
   flow = interface.findFlowByKey(tonumber(flow_key))
end

sendHTTPHeader('text/html')
--sendHTTPHeader('application/json')

if(flow == nil) then
   print('{}')
else
   diff0 = os.time()-flow["seen.first"]
   diff = os.time()-flow["seen.last"]
   thpt = 8*flow["throughput"]
   print('{ ' .. '"seen.last": "'.. formatEpoch(flow["seen.last"]) .. ' ['.. secondsToTime(diff) .. ' ago]", ' 
   .. '"seen.first": "'.. formatEpoch(flow["seen.first"]) .. ' ['.. secondsToTime(diff0) .. ' ago]"' 
   .. ', "bytes": ' .. flow["bytes"] .. ', "cli2srv.packets": ' .. flow["cli2srv.packets"] .. ', "srv2cli.packets": ' .. flow["srv2cli.packets"] .. ', "cli2srv.bytes": ' .. flow["cli2srv.bytes"] .. ', "srv2cli.bytes": ' .. flow["srv2cli.bytes"].. ', "throughput": "' .. bitsToSize(thpt)..'", "throughput_raw": ' .. thpt..' }\n')
end
