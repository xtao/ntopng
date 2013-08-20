--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('application/json')

tracked_host = _GET["host"]

interface.find(ifname)
peers = interface.getFlowPeers()

max_num_links = 32
max_num_hosts = 8

-- 1. compute total traffic
total_traffic = 0
for key, values in pairs(peers) do
   total_traffic = total_traffic + values["sent.last"] + values["rcvd.last"]
   -- print("->"..key.."(".. total_traffic..")" .. "\n")
end

-- 2. compute flow threshold under which we do not print any relation
if(tracked_host == nil) then
   threshold = (total_traffic * 3) / 100
else
   threshold = 1
end

hosts = {}
num = 0
print '{"nodes":[\n'

while(num == 0) do
   for key, values in pairs(peers) do
      if((values["sent.last"] + values["rcvd.last"]) > threshold) then

	 --print("[" .. key .. "][" .. values["client"] .. "][" .. values["server"] .. "][" .. tracked_host .. "]\n")
	 if((tracked_host == nil) or findString(key, tracked_host) or findString(values["client"], tracked_host) or findString(values["server"], tracked_host)) then
	    -- print("[" .. key .. "][" .. tracked_host .. "]\n")

	    for key,word in pairs(split(key, " ")) do
	       if(num >= max_num_hosts) then
		  break
	       end

	       if(hosts[word] == nil) then
		  hosts[word] = num

		  if(num > 0) then
		     print ",\n"
		  end
		  -- 3. print nodes
		  print ("\t{\"name\": \"" .. ntop.getResolvedAddress(word) .. "\", \"ip\": \"" .. word .. "\"}")
		  num = num + 1
	       end
	    end
	 end
      end
   end

   if(num == 0) then
      -- Lower the threshold to hope finding hosts
      threshold = threshold / 2
   end

   if(threshold <= 1) then
      break
   end
end

top_host = nil
top_value = 0

if(num == 0) then
   -- 2.1 It looks like in this network there are many flows with no clear predominant traffic
   --     Then we take the host with most traffic and print flows belonging to it

   hosts_stats = interface.getHosts()
   for key, value in pairs(hosts_stats) do
      if(value > top_value) then
	 top_host = key
	 top_value = value
      end -- if
   end -- for

   if(top_host ~= nil) then
      -- We now have have to find this host and some peers
      hosts[top_host] = 0
      print ("{\"name\": \"" .. top_host .. "\"}")
      num = num + 1

      for key, values in pairs(peers) do
	 if(findString(key, top_host) or findString(values["client"], top_host) or findString(values["server"], top_host)) then
	    for key,word in pairs(split(key, " ")) do
	       if(hosts[word] == nil) then
		  hosts[word] = num

		  -- 3. print nodes
		  print (",\n{\"name\": \"" .. word .. "\"}")
		  num = num + 1

		  if(num >= max_num_hosts) then
		     break
		  end
	       end --if

	       if(num >= max_num_hosts) then
		  break
	       end

	    end -- for
	 end -- if

	 if(num >= max_num_hosts) then
	    break
	 end
      end -- for
   end -- if
end -- if


print "\n],\n"
print '"links" : [\n'

-- 4. print links
--  print (top_host)
num = 0

-- Avoid to have a link A->B, and B->A
reverse_nodes = {}
for key, values in pairs(peers) do
   val = values["sent.last"] + values["rcvd.last"]

   if((val > threshold) or ((top_host ~= nil) and (findString(key, top_host) ~= nil)) and (num < max_num_links)) then
      e = {}
      id = 0
      --print("->"..key.."\n")
      for key,word in pairs(split(key, " ")) do
	 --print(word .. "=" .. hosts[word].."\n")
	 e[id] = hosts[word]
	 id = id + 1
      end

      if((e[0] ~= nil) and (e[1] ~= nil) and (e[0] ~= e[1]) and (reverse_nodes[e[0]..":"..e[1]] == nil)) then
	 if(num > 0) then
	    print ",\n"
	 end

	 reverse_nodes[e[1]..":"..e[0]] = 1

	 sentv = values["sent.last"]
	 recvv = values["rcvd.last"]
	 
	 if(val == 0) then
	    val = 1
	 end

	 print ("\t{\"source\": " .. e[0] .. ", \"target\": " .. e[1] .. ", \"value\": " .. val .. ", \"sent\": " .. sentv .. ", \"rcvd\": ".. recvv .. "}")
	 num = num + 1
      end
   end

end



print ("\n]}\n")


