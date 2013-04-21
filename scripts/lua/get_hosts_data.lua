ifname      = _GET["if"]
currentPage = _GET["currentPage"]
perPage     = _GET["perPage"]
columnSort  = _GET["sort"]

if(currentPage == nil) then
   currentPage = 1
else
   currentPage = tonumber(currentPage)
end

if(perPage == nil) then
   perPage = 1
else
   perPage = tonumber(perPage)
end


if(ifname == nil) then	  
  ifname = "any"
end

interface.find(ifname)
hosts_stats = interface.getHostsInfo()

print ("{ \"currentPage\" : " .. currentPage .. ",\n \"data\" : [\n")
num = 0
total = 0
to_skip = (currentPage-1) * perPage
for key, value in pairs(hosts_stats) do
   if(to_skip > 0) then
      to_skip = to_skip-1
   else
      if(num < perPage) then
	 if(num > 0) then
	    print ",\n"
	 end
	 
	 print ("{ \"column_0\" : \"<A HREF='/host_details.lua?interface=".. ifname .. "&host=" .. key .. "'>" .. key .. "</A>\", \"column_1\" : \"" .. value["name"] .. "\", \"column_2\" : \"" .. (value["bytes.sent"]+value["bytes.rcvd"]) .. "\" } ")
	 num = num + 1
      end
   end

   total = total + 1
end -- for


print ("\n], \"perPage\" : " .. perPage .. ",\n")
print ("\"sort\" : [ [ \"column_0\", \"desc\" ] ],\n \"totalRows\" : " .. total .. " \n}")
