--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "flow_utils"

flow_key = _GET["flow_key"]
if(flow_key == nil) then
   flow = nil
else
   interface.find(ifname)
   flow = interface.findFlowByKey(tonumber(flow_key))
end

throughput_type = getThroughputType()

sendHTTPHeader('text/html')
--sendHTTPHeader('application/json')

if(flow == nil) then
   print('{}')
else

   diff0 = os.time()-flow["seen.first"]
   diff = os.time()-flow["seen.last"]
   -- Default values
   thpt = 0
   thpt_display = bitsToSize(0)
   if (throughput_type == "bps") then
      thpt = 8*flow["throughput_bps"]
      thpt_display = bitsToSize(thpt)
   elseif (throughput_type == "pps") then
      thpt = flow["throughput_pps"]
      thpt_display = pktsToSize(thpt)
   end
   print('{ ' .. '"seen.last": "'.. formatEpoch(flow["seen.last"]) .. ' ['.. secondsToTime(diff) .. ' ago]", ' 
   .. '"seen.first": "'.. formatEpoch(flow["seen.first"]) .. ' ['.. secondsToTime(diff0) .. ' ago]"' 
   .. ', "bytes": ' .. flow["bytes"] .. ', "cli2srv.packets": ' .. flow["cli2srv.packets"] .. ', "srv2cli.packets": ' .. flow["srv2cli.packets"] .. ', "cli2srv.bytes": ' .. flow["cli2srv.bytes"] .. ', "srv2cli.bytes": ' .. flow["srv2cli.bytes"].. ', "throughput": "' .. thpt_display..'", "throughput_raw": ' .. thpt..' }\n')
end
