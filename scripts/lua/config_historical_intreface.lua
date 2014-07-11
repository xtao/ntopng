--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('application/html')
err = ""


interface_id = _GET["id"]
from = _GET["from"]
to = _GET["to"]


if ((from ~= nil) and (to ~= nil) and (interface_id ~= nil)) then

  ntop.startHistoricalInterface(tonumber(from), tonumber(to), interface.name2id(interface_id))

end

if (err == "")  then
  print "{ \"result\" : \"0\"}";
else
  print ( "{ \"result\" : \"-1\", \"description\" : \"" .. err .."\" }" );
end
