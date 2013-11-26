--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"


function cleanName(name)
   n = string.gsub(name, "'", "_")
   -- Cut the name at 128 chars
   n = string.sub(n, 1, 128)
   return(n)
end

sendHTTPHeader('text/html')

t = os.time() -- -86400
when = os.date("%y%m%d", t)
key_name = when..".keys"

--print(key_name.."\n")


local delete_keys = false

dump_dir = fixPath(dirs.workingdir .. "/datadump/")
ntop.mkdir(dump_dir)

fname = dump_dir .. "20".. when ..".sql"
out = io.open(fname, "w")

print("Please wait: we are dumping data into ".. fname .."\n")

out:write("CREATE DATABASE IF NOT EXISTS `20"..when.."`;\nUSE `20"..when.."`;\n")
out:write("\nCREATE TABLE IF NOT EXISTS `activities` (\n`idx` int(11) NOT NULL,\n`interface` varchar(8) NOT NULL,\n`name` varchar(128) NOT NULL,\n`type` enum('aggregations','host_contacts') NOT NULL,\nPRIMARY KEY (`idx`)\n) ENGINE=InnoDB DEFAULT CHARSET=latin1;\n\nCREATE TABLE IF NOT EXISTS `contacts` (\n`idx` int(11) NOT NULL AUTO_INCREMENT,\n`contacts_idx` int(11) NOT NULL,\n`mode` enum('contacted_by','contacted_peers') NOT NULL,\n`name` varchar(128) NOT NULL,\n`family` mediumint(11) NOT NULL,\n`num_contacts` int(11) NOT NULL,\nPRIMARY KEY (`idx`)\n) ENGINE=InnoDB DEFAULT CHARSET=latin1;\n")

idx = 0
repeat
   key = ntop.setPopCache(key_name)

   if(key == "") then break end

   k1 = when.."|"..key.."|contacted_by"
   v1 = ntop.getHashKeysCache(k1)
   if(v1 ~= nil) then
      res = split(k1, "|")

      out:write("INSERT INTO `activities` (`idx`, `interface`, `name`, `type`) VALUES ('".. idx.."', '".. res[3].."', '".. cleanName(res[4]).."', '".. res[2].."');\n")

      --print("->"..k1.."\n")
      for k,_ in pairs(v1) do
	 v = ntop.getHashCache(k1, k)
	 res = split(k, "@")
	 --print("\t"..k .. "=" .. v.. "\n")
	 if((res[1] ~= nil) and (res[2] ~= nil) and (v ~= nil)) then
	    out:write("INSERT INTO `contacts` (`contacts_idx`, `mode`, `name`, `family`, `num_contacts`) VALUES (".. idx..", 'contacted_by', '"..cleanName(res[1]).."', ".. res[2]..", ".. v..");\n")
	 end
      end

      idx = idx + 1
      if(delete_keys) then ntop.delHashCache(k1) end
   end


   k2 = when.."|"..key.."|contacted_peers"
   v2 = ntop.getHashKeysCache(k2)
   if(v2 ~= nil) then
      res = split(k1, "|")
      out:write("INSERT INTO `activities` (`idx`, `interface`, `name`, `type`) VALUES ('".. idx.."', '".. res[3].."', '".. res[4].."', '".. res[2].."');\n")

      --print("->"..k2.."\n")
      for k,v in pairs(v2) do
	 v = ntop.getHashCache(k2, k)
	 res = split(k, "@")
	 --print("\t"..k .. "=" .. v.. "\n")
	 if((res[1] ~= nil) and (res[2] ~= nil) and (v ~= nil)) then
	    out:write("INSERT INTO `contacts` (`contacts_idx`, `mode`, `name`, `family`, `num_contacts`) VALUES ('".. idx.."', 'contacted_peers', '"..cleanName(res[1]).."', ".. res[2]..", ".. v..");\n")
	 end
      end

      idx = idx + 1
      if(delete_keys) then ntop.delHashCache(k2) end
   end

   until(key == "")


out:close()

print("\nDone.\n")