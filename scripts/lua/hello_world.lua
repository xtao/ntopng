--
-- (C) 2013 - ntop.org
--

-- Hello world

package.path = "./scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"

sendHTTPHeader('text/html')

print('<html><head><title>ntop</title></head><body>Hello ' .. os.date("%d.%m.%Y"))

-- Print _GET variable
for key, value in pairs(_GET) do 
   print(key.."="..value.."<p>\n")
end


iface = interface.find("any") -- put real interface name here
stats = interface.getStats()

print(stats["packets"])
print(stats["bytes"])
print('</body></html>\n')



