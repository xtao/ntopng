--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('application/html')
err = ""

interface_id = _GET["id"]
from = tonumber(_GET["from"])
to = tonumber(_GET["to"])
epoch = tonumber(_GET["epoch"])

ret = 1

if ((from ~= nil) and (to ~= nil) and (interface_id ~= nil)) then

  actual_epoch = from
  actual_to = to - 300
  i = 0
  id = interface.name2id(interface_id)

  while (actual_epoch <= actual_to) do

    file_name = dirs.workingdir .. '/' .. id .. '/flows' .. os.date("/%Y/%m/%d/%H/%M",actual_epoch) .. '.sqlite'

    if (i == 0) then
      ret = interface.loadHistoricalFile(file_name,true)
      interface.setHistorical(from,to,id)
    else
      ret = interface.loadHistoricalFile(file_name)
    end

    if (ret == 0) then err = err .. "Missing file: "..file_name.."</br>" end
    if (ret == -1) then err = err .. "Impossible open file: " .. file_name .."<br>" end

    actual_epoch = actual_epoch + 300
    i = i + 1
  end

end

if (err == "")  then
  print "{ \"result\" : \"0\"}";
else
  print ( "{ \"result\" : \"-1\", \"description\" : \"" .. err .."\" }" );
end
