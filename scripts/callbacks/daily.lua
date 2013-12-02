--
-- (C) 2013 - ntop.org
--


dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

local debug = false
local delete_keys = true

begin = os.clock()
t = os.time() -86400
when = os.date("%y%m%d", t)
key_name = when..".keys"

--print(key_name.."\n")

dump_dir = fixPath(dirs.workingdir .. "/datadump/")
ntop.mkdir(dump_dir)

db =  sqlite3.open(dump_dir.."20"..when..".sqlite")
db:exec[[
      CREATE TABLE IF NOT EXISTS `interfaces` (`idx` INTEGER PRIMARY KEY AUTOINCREMENT, `interface_name` STRING);
      CREATE TABLE IF NOT EXISTS `hosts` (`idx` INTEGER PRIMARY KEY AUTOINCREMENT, `host_name` STRING);
      CREATE TABLE IF NOT EXISTS `activities` (`idx` INTEGER PRIMARY KEY AUTOINCREMENT, `interface_idx` INTEGER, `host_idx` INTEGER, `type` INTEGER);
      CREATE TABLE IF NOT EXISTS `contacts` (`idx` INTEGER PRIMARY KEY AUTOINCREMENT, `activity_idx` INTEGER KEY, `contact_type` INTEGER, `host_idx` INTEGER KEY, `contact_family` INTEGER, `num_contacts` INTEGER);
]]

db:exec('INSERT INTO contacts(idx, activity_idx, contact_type, host_idx, contact_family, num_contacts) VALUES (339,0,0,1,65535,34);')
db:exec('INSERT INTO activities(idx, interface_idx, host_idx, type) VALUES (213,0,156,0);')

-- #########################

function cleanName(name)
   n = string.gsub(name, "'", "_")
   n = string.gsub(n, ",", "_")
   -- Cut the name at 128 chars
   n = string.sub(n, 1, 128)
   return(n)
end

-- #########################  

interfaces_id     = 0
interfaces_hash   = { }
-- Interfaces are a few so we still cache them
function interface2id(name)
   if(interfaces_hash[name] == nil) then
      id = interfaces_id
      interfaces_hash[name] = id
      db:exec('INSERT INTO interfaces(idx, interface_name) VALUES ('..id..', "'.. name .. '");')
      interfaces_id = interfaces_id + 1
      return(id)
   else
      return(interfaces_hash[name])
   end
end

-- #########################

hosts_id = 0
function host2id(name)
   name = cleanName(name)

   for idx in db:urows('SELECT idx from hosts where host_name="'..name..'"')
   do return(idx) end

   db:exec('INSERT INTO hosts(idx, host_name) VALUES ('..hosts_id..', "'.. name .. '");')
   hosts_id = hosts_id + 1
   return(hosts_id)
end

-- #########################

function add_to_activities(a, b, c, d)
   sql = 'INSERT INTO activities(idx, interface_idx, host_idx, type) VALUES ('..a..','..b..','..c..','..d..');'
   if(debug) then print(sql.."\n") end
   db:exec(sql)
end

-- #########################

function add_to_contacts(a, b, c, d, e, f)
   sql = 'INSERT INTO contacts(idx, activity_idx, contact_type, host_idx, contact_family, num_contacts) VALUES ('..a..','..b..','..c..','..d..','..e..','..f..');'
   if(debug) then print(sql.."\n") end
   db:exec(sql)
end

-- #########################

if(debug) then sendHTTPHeader('text/html') end

contact_idx = 0
idx = 0
repeat
   key = ntop.setPopCache(key_name)
   if(debug) then print("====> "..key_name.."\n") end
   if((key == nil) or (key == "")) then break end

   if(debug) then print("=> "..key.."\n") end
   k1 = when.."|"..key.."|contacted_by"
   v1 = ntop.getHashKeysCache(k1)
   if(v1 ~= nil) then
      res = split(k1, "|")

      if(res[2] == "host_contacts") then 
	 r = 0 
      else
	 -- aggregations
	 r = 1 
      end
      name = host2id(res[4])
      add_to_activities(idx,interface2id(res[3]),name,r)

      if(debug) then print("-> (1)"..k1.."\n") end
      for k,_ in pairs(v1) do
	 v = ntop.getHashCache(k1, k)
	 res = split(k, "@")
	 if(debug) then print("\t"..k .. "=" .. v.. "\n") end
	 if((res[1] ~= nil) and (res[2] ~= nil) and (v ~= nil)) then
	    add_to_contacts(contact_idx,id,0,host2id(res[1]),res[2],v)
	    contact_idx = contact_idx + 1
	 end
	 if(delete_keys) then ntop.delHashCache(k1, k) end
      end

      idx = idx + 1
   end

   k2 = when.."|"..key.."|contacted_peers"
   v2 = ntop.getHashKeysCache(k2)
   if(v2 ~= nil) then
      res = split(k1, "|")

      if(res[2] == "host_contacts") then 
	 r = 0 
      else
	 -- aggregations
	 r = 1 
      end
      name = host2id(res[4])
      add_to_activities(idx,interface2id(res[3]),name,r)

      if(debug) then print("-> (2)"..k2.."\n") end
      for k,v in pairs(v2) do
	 v = ntop.getHashCache(k2, k)
	 res = split(k, "@")
	 if(debug) then print("\t"..k .. "=" .. v.. "\n") end
	 if((res[1] ~= nil) and (res[2] ~= nil) and (v ~= nil)) then
	    add_to_contacts(contact_idx,idx,1,host2id(res[1]),res[2],v)
	    contact_idx = contact_idx + 1
	 end
	 if(delete_keys) then ntop.delHashCache(k2, k) end
      end

      idx = idx + 1
   end

   until(key == "")

db:close()

sec = os.clock() - begin
print(string.format("Elapsed time: %.2f min\n", sec/60).."\n")
print("\nDone.\n")

-- redis-cli KEYS "131129|*" | xargs redis-cli DEL
