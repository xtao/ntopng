ifname = _GET["if"]
interface.find("any")
peers = interface.getFlowPeers()

max_num_links = 32
max_num_hosts = 8

-- 1. compute total traffic
total_traffic = 0
for key, values in pairs(peers) do
   total_traffic = total_traffic + values["sent"] + values["rcvd"]
end

-- 2. compute flow threshold under which we do not print any relation
threshold = (total_traffic * 3) / 100

hosts = {}
num = 0
print '{"nodes":[\n'
for key, values in pairs(peers) do
   if((values["sent"] + values["rcvd"]) > threshold) then
      for word in string.gmatch(key, "[%d+.]+") do
	 if(hosts[word] == nil) then
	    hosts[word] = num

	    if(num > 0) then
	       print ",\n"
	    end
	    -- 3. print nodes
	    print ("{\"name\": \"" .. word .. "\"}")
	    num = num + 1

	    if(num >= max_num_hosts) then
	       break
	    end
	 end
      end
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
	 if(string.find(key, top_host) >= 0) then
	    for word in string.gmatch(key, "[%d+.]+") do
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
for key, values in pairs(peers) do
   val = values["sent"] + values["rcvd"]
   --print(key.."\n")

   if((val > threshold) or ((top_host ~= nil) and (string.find(key, top_host) >= 0)) and (num < max_num_links)) then
      e = {}
      id = 0
      for word in string.gmatch(key, "[%d+.]+") do
	 -- print(word .. "=" .. hosts[word].."\n")
	 e[id] = hosts[word]
	 id = id + 1
      end
      
      if((e[0] ~= nil) and (e[1] ~= nil) and (e[0] ~= e[1])) then
	 if(num > 0) then
	    print ",\n"
	 end
	 
	 print ("{\"source\": " .. e[0] .. ", \"target\": " .. e[1] .. ", \"value\": " .. val .. ", \"sent\": " .. values["sent"] .. ", \"rcvd\": ".. values["rcvd"] .. "}")
	 num = num + 1
      end
   end

end



print ("\n]}\n")




