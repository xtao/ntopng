--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('application/html')

interface_id = _GET["id"]
from = tonumber(_GET["from"])
to = tonumber(_GET["to"])
epoch = tonumber(_GET["epoch"])

ret = false

if ((from ~= nil) and (to ~= nil) and (interface_id ~= nil)) then
  id = interface.name2id(interface_id)
  ret = interface.loadHistoricalInterval(from,to,id)
end

if (ret)  then
  print "{ \"result\" : \"0\"}";
else
  print ( "{ \"result\" : \"-1\"}" );
end
