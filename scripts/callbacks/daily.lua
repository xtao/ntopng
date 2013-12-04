--
-- (C) 2013 - ntop.org
--


dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

local debug = false
local delete_keys = true

begin = os.clock()
t = os.time()-86400

if((_GET ~= nil) and (_GET["debug"] ~= nil)) then
   debug = true
   t = t + 86400
end

if(debug) then sendHTTPHeader('text/plain') end

when = os.date("%y%m%d", t)
key_name = when..".keys"

--print(key_name.."\n")

dump_dir = fixPath(dirs.workingdir .. "/datadump/")
ntop.mkdir(dump_dir)

db_name = dump_dir.."20"..when..".sqlite"
print("Opening DB: "..db_name.."\n")
db =  sqlite3.open(db_name)

db:exec[[
      CREATE TABLE IF NOT EXISTS `interfaces` (`idx` INTEGER PRIMARY KEY AUTOINCREMENT, `interface_name` STRING);
      CREATE TABLE IF NOT EXISTS `hosts` (`idx` INTEGER PRIMARY KEY AUTOINCREMENT, `host_name` STRING KEY);
      CREATE TABLE IF NOT EXISTS `activities` (`idx` INTEGER PRIMARY KEY AUTOINCREMENT, `interface_idx` INTEGER, `host_idx` INTEGER, `type` INTEGER);
      CREATE TABLE IF NOT EXISTS `contacts` (`idx` INTEGER PRIMARY KEY AUTOINCREMENT, `activity_idx` INTEGER KEY, `contact_type` INTEGER, `host_idx` INTEGER KEY, `contact_family` INTEGER, `num_contacts` INTEGER);
      BEGIN;
]]

num_db_operations = 0 

-- #########################

function execQuery(where, sql)
   if(debug) then print("[SQL] (".. num_db_operations .. ") "..sql.."\n") end
   if(where:exec(sql)  ~= sqlite3.OK) then	  
      print("SQLite ERROR: ".. where:errmsg() .. " [" .. sql .. "]\n")
   end

   num_db_operations = num_db_operations + 1

   if(num_db_operations >= 100) then
      num_db_operations = 0
      execQuery(where, "COMMIT; BEGIN;");
   end
end

-- #########################

function cleanName(name)
   -- print(name.."\n")
   n = string.gsub(name, "'", "_")
   n = string.gsub(n, ",", "_")
   n = string.gsub(n, '"', "_")
   -- Cut the name at 128 chars
   n = string.sub(n, 1, 128)
   return(n)
end

-- #########################  

-- Interfaces are a few so we still cache them
interfaces_id     = 0
interfaces_hash   = { }

function interface2id(name)
   if(interfaces_hash[name] == nil) then
      id = interfaces_id
      interfaces_hash[name] = id
      execQuery(db, 'INSERT INTO interfaces(idx, interface_name) VALUES ('..id..', "'.. name .. '");')
      interfaces_id = interfaces_id + 1
      return(id)
   else
      return(interfaces_hash[name])
   end
end

-- #########################

host_id = 0
function host2id(name)
   name = cleanName(name)

   for idx in db:urows('SELECT idx FROM hosts WHERE host_name="'..name..'"')
   do return(idx) end

   execQuery(db, 'INSERT INTO hosts(idx, host_name) VALUES ('..host_id..', "'.. name .. '");')
   id = host_id
   host_id = host_id + 1
   return(id)
end

-- #########################

function add_to_activities(a, b, c, d)
   sql = 'INSERT INTO activities(idx, interface_idx, host_idx, type) VALUES ('..a..','..b..','..c..','..d..');'
   execQuery(db, sql)
end

-- #########################

function add_to_contacts(a, b, c, d, e, f)
   sql = 'INSERT INTO contacts(idx, activity_idx, contact_type, host_idx, contact_family, num_contacts) VALUES ('..a..','..b..','..c..','..d..','..e..','..f..');'
   execQuery(db, sql)
end

-- #########################

-- Flush all hosts to disk
ifnames = interface.getIfNames()
for _,_ifname in pairs(ifnames) do
   interface.find(purifyInterfaceName(_ifname))
   interface.flushHostContacts()
end

-- #########################

keyset = ntop.getMembersCache(key_name)

num_ifaces = 0
contact_id = 0
activity_id = 0
for _,key in ipairs(keyset) do
   -- if(debug) then print("====> "..key_name.."\n") end
   if((key == nil) or (key == "")) then break end

   if(debug) then print("\n\n=> "..key.."\n") end
   k1 = when.."|"..key.."|contacted_by"
   v1 = ntop.getHashKeysCache(k1)
   if(v1 ~= nil) then
      res = split(k1, "|")
      
      if(#res ~= 5) then
	 print("Invalid string ["..k1.."]")
      else
	 if(res[2] == "host_contacts") then 
	    r = 0 
	 else
	    -- aggregations
	    r = 1 
	 end
	 name = host2id(res[4])
	 add_to_activities(activity_id,interface2id(res[3]),name,r)
	 
	 if(debug) then print("-> (1)"..k1.."\n") end
	 for k,_ in pairs(v1) do
	    v = ntop.getHashCache(k1, k)
	    res = split(k, "@")

	    if(#res ~= 2) then
	       print("Invalid string ["..k.."]")
	    else
	       if(debug) then print("\t"..k .. "=" .. v.. "\n") end
	       if((res[1] ~= nil) and (res[2] ~= nil) and (v ~= nil)) then
		  add_to_contacts(contact_id,activity_id,0,host2id(res[1]),res[2],v)
		  contact_id = contact_id + 1
	       end
	       if(delete_keys) then ntop.delHashCache(k1, k) end
	    end
	 end
	 
	 activity_id = activity_id + 1
      end
   end

   k2 = when.."|"..key.."|contacted_peers"
   v2 = ntop.getHashKeysCache(k2)
   if(v2 ~= nil) then
      res = split(k1, "|")

      if(#res ~= 5) then
	 print("Invalid string ["..k1.."]")
      else
	 if(res[2] == "host_contacts") then 
	    r = 0 
	 else
	    -- aggregations
	    r = 1 
	 end
	 name = host2id(res[4])
	 add_to_activities(activity_id,interface2id(res[3]),name,r)

	 if(debug) then print("-> (2)"..k2.."\n") end
	 for k,v in pairs(v2) do
	    v = ntop.getHashCache(k2, k)
	    res = split(k, "@")

	    if(#res ~= 2) then
	       print("Invalid string ["..k.."]")
	    else
	       if(debug) then print("\t"..k .. "=" .. v.. "\n") end
	       if((res[1] ~= nil) and (res[2] ~= nil) and (v ~= nil)) then
		  add_to_contacts(contact_id,activity_id,1,host2id(res[1]),res[2],v)
		  contact_id = contact_id + 1
	       end
	       if(delete_keys) then ntop.delHashCache(k2, k) end
	    end
	 end

	 activity_id = activity_id + 1
      end
   end
end

ntop.delCache(key_name)

execQuery(db, 'COMMIT;')
db:close()

sec = os.clock() - begin
print(string.format("Elapsed time: %.2f min\n", sec/60).."\n")
print(interfaces_id .. " interfaces processed\n")
print(host_id .. " hosts processed\n")
print(contact_id .. " contacts processed\n")
print(activity_id .. " activities processed\n")
print("\nDone.\n")

-- redis-cli KEYS "131129|*" | xargs redis-cli DEL
