--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')

currentPage = 1;
perPage     = _GET["perPage"]

if(perPage == nil) then
   perPage = 10
else
   perPage = tonumber(perPage)
end

alerts = ntop.getQueuedAlerts(perPage)

print ("{ \"currentPage\" : " .. currentPage .. ",\n \"data\" : [\n")
total = 0

for _key, _value in pairs(alerts) do
   if(total > 0) then print(",\n") end
   values = split(string.gsub(_value, "\n", ""), "|")
   column_date = values[1]
   column_severity = values[2]
   column_type = values[3]
   column_msg = values[4]

   print('{ "column_key" : "Info", "column_date" : "'..column_date..'", "column_severity" : "'..column_severity..'", "column_type" : "'..column_type..'", "column_msg" : "'..column_msg..'" }')

   total = total + 1
end -- for


print ("\n], \"perPage\" : " .. perPage .. ",\n")

print ("\"sort\" : [ [ \"\", \"\" ] ],\n")
print ("\"totalRows\" : " .. total .. " \n}")
