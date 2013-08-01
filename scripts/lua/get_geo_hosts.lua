--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')

ifname = _GET["if"]
interface.find("any")
peers = interface.getFlowPeers()



print [[

{"center":[0, 0],
"objects":
	[

     ]]

maxval = 0
for key, values in pairs(peers) do
   t = values["sent"]+values["rcvd"]

   if(t > maxval) then maxval = t end
end

min_threshold = (t*10)/100
max_num = 100
num = 0
for key, values in pairs(peers) do
   if(((values["client.latitude"]+values["client.longitude"]) > 0) and ((values["server.latitude"]+values["server.longitude"]) > 0)) then
      t = values["sent"]+values["rcvd"]
      pctg = (t*100)/maxval

      if(pctg >= min_threshold) then 
	 if(num > 0) then print(",") end
	 print('{\n"host":\n[	\n{\n')
	 print('"lat": '..values["client.latitude"]..',\n')
	 print('"lng": '..values["client.longitude"]..',\n')
	 print('"html": "City: '..values["client.city"]..'",\n')
	 print('"name": "'..values["client"]..'"\n')
	 print('},\n{\n')
	 print('"lat": '..values["server.latitude"]..',\n')
	 print('"lng": '..values["server.longitude"]..',\n')
	 print('"html": "City: '..values["server.city"]..'",\n')
	 print('"name": "'..values["server"]..'"\n')
	 print('}\n],\n"flusso": '.. pctg..',"html":"Flow '.. key .. '"\n')
	 print('}\n')
	 num = num + 1
	 
	 if(num > max_num) then break end
      end
   end
end


print [[
	]
}

]]