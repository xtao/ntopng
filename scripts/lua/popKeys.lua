--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/plain')

t = os.time() -- -86400
when = os.date("%y%m%d", t)
key_name = when..".keys"

print(key_name.."\n")

repeat
   key = ntop.setPopCache(key_name)

   if(key == "") then break end

   k1 = when.."|"..key.."|contacted_by"
   v1 = ntop.getHashKeysCache(k1)
   if(v1 ~= nil) then
      print("->"..k1.."\n")
      for k,_ in pairs(v1) do
	 v = ntop.getHashCache(k1, k)
	 print("\t"..k .. "=" .. v.. "\n")
      end

      ntop.delHashCache(k1)
   end


   k2 = when.."|"..key.."|contacted_peers"
   v2 = ntop.getHashKeysCache(k2)
   if(v2 ~= nil) then
      print("->"..k2.."\n")
      for k,v in pairs(v2) do
	 v = ntop.getHashCache(k2, k)
	 print("\t"..k .. "=" .. v.. "\n")
      end

      ntop.delHashCache(k2)
   end

   until(key == "")