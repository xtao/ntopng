--
-- (C) 2013 - ntop.org
--


function getTopTalkers(ifname)
rsp = ""

if(ifname == nil) then ifname = "any" end

interface.find(ifname)
hosts_stats = interface.getFlowsInfo()

sent = {}
_sent = {}
rcvd = {}
_rcvd = {}

for _key, value in pairs(hosts_stats) do
   key = hosts_stats[_key]["src.ip"]   
   old = _sent[key]
   if(old == nil) then old = 0 end
   _sent[key] = old + hosts_stats[_key]["cli2srv.bytes"]

   key = hosts_stats[_key]["dst.ip"]
   old = _rcvd[key]
   if(old == nil) then old = 0 end
   _rcvd[key] = old + hosts_stats[_key]["cli2srv.bytes"]

   -- ###########################

   key = hosts_stats[_key]["dst.ip"]   
   old = _sent[key]
   if(old == nil) then old = 0 end
   _sent[key] = old + hosts_stats[_key]["srv2cli.bytes"]

   key = hosts_stats[_key]["src.ip"]
   old = _rcvd[key]
   if(old == nil) then old = 0 end
   _rcvd[key] = old + hosts_stats[_key]["srv2cli.bytes"]
end

for key, value in pairs(_sent) do
   sent[value] = key
end

for key, value in pairs(_rcvd) do
   rcvd[value] = key
end


num = 0
rsp = rsp .. "{\n"
rsp = rsp .. '\t"senders": ['
for _key, _value in pairsByKeys(sent, rev) do
   key   = sent[_key]
   value = _key

   if(value == 0) then break end
   if(num > 0) then rsp = rsp .. " }," end
   rsp = rsp .. '\n\t\t { "'..key..'": '..value
   num = num + 1

   if(num == 10) then
      break
   end
end

rsp = rsp .. " }\n\t],\n"
rsp = rsp .. '\t"receivers": ['

num = 0
for _key, _value in pairsByKeys(rcvd, rev) do
   key   = rcvd[_key]
   value = _key

   if(value == 0) then break end	
   if(num > 0) then rsp = rsp .. " }," end
   rsp = rsp .. '\n\t\t { "'..key..'": '..value
   num = num + 1

   if(num == 10) then
      break
   end
end

rsp = rsp .. " }\n\t]\n}\n"

return(rsp)
end