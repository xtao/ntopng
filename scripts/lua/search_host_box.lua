--
-- (C) 2013 - ntop.org
--

print [[
	 <li><form action="/host_details.lua"><input type="text" name="host" class="search-query span2" placeholder="Search Host" data-provide="typeahead" data-items="4" data-source=[]]

ifname = _GET["if"]

if(ifname == nil) then
  ifname = "any"
end

interface.find(ifname)
hosts_stats = interface.getHostsInfo()

num = 0
for key, value in pairs(hosts_stats) do
   if(num > 0) then
      print (",")
   end
   print ('"'.. key .. '","' .. hosts_stats[key]["name"] .. '"')
  num = num + 1
end
	 print [[]></form></li>
   ]]

