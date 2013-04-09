ifname = _GET["if"]
interface.find("any")
peers = interface.getFlowPeers()


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
	 end
      end
   end
end

print "\n],\n"
print '"links" : [\n'

-- 4. print links

num = 0
for key, values in pairs(peers) do
   val = values["sent"] + values["rcvd"]
   
   if(val > threshold) then
      e = {}
      id = 0
      for word in string.gmatch(key, "[%d+.]+") do
	 --print(word .. "=" .. hosts[word].."\n")
	 e[id] = hosts[word]
	 id = id + 1
      end
      
      if(num > 0) then
	 print ",\n"
      end
      
      print ("{\"source\": " .. e[0] .. ", \"target\": " .. e[1] .. ", \"value\": " .. val .. "}")
      num = num + 1
   end
end


print ("\n]}\n")




