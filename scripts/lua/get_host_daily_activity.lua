--
-- (C) 2013 - ntop.org
--

local debug = true

function host2id(d, name)
   sql = 'SELECT idx FROM hosts WHERE host_name="'..name..'"'
   if(debug) then print(sql.."\n") end
   for idx in d:urows(sql) do return(tonumber(idx)) end

   return nil
end

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "graph_utils"

host_ip = _GET["host"]
mode = _GET["mode"]
family = _GET["family"]


t = os.time() -- -86400
when = os.date("%y%m%d", t)
dump_dir = fixPath(dirs.workingdir .. "/datadump/")
db_name = dump_dir.."20"..when..".sqlite"

sendHTTPHeader('application/json')

-- print(db_name.."\n")

max_num = 250

db = sqlite3.open(db_name)
if((db == nil) or (host_ip == nil)) then
   print("{ }\n")
else
   interface.find(ifname)
   
   if(mode == "overview") then
      contact_type = 0 -- contacted_by
      contact_type = 1 -- contacted_peers
      
      sql = 'select contacts.contact_type,hosts.host_name,sum(contacts.num_contacts) as tot from contacts,hosts where contacts.activity_idx in (select activities.idx from activities,hosts where activities.host_idx=hosts.idx and hosts.host_name="'..host_ip..'") and contacts.contact_family !=254 and contacts.host_idx=hosts.idx group by contacts.contact_type,hosts.host_name order by tot desc,contacts.contact_type,hosts.host_name limit '..max_num
      
      for contact_type,host_name,num_contacts in db:urows(sql) do 
	 print(contact_type..",'"..host_name..","..num_contacts.."\n")
      end
      elseif(mode == "overview_1") then
      sql = 'select contacts.contact_type,hosts.host_name,contacts.contact_family,sum(contacts.num_contacts) as tot from contacts,hosts where contacts.activity_idx in (select activities.idx from activities,hosts where activities.host_idx=hosts.idx and hosts.host_name="'..host_ip..'") and contacts.host_idx=hosts.idx group by contacts.contact_type,contacts.contact_family,hosts.host_name order by tot desc,contacts.contact_type,hosts.host_name limit '..max_num
      
      for contact_type,host_name,contact_family,num_contacts in db:urows(sql) do 
	 print(contact_type..",'"..host_name.."',"..contact_family..","..num_contacts.."\n")
      end
      elseif(mode == "overview_family") then
      if(family == nil) then family = 5 end -- DNS
      sql = 'select contacts.contact_type,hosts.host_name,sum(contacts.num_contacts) as tot from contacts,hosts where contacts.activity_idx in (select activities.idx from activities,hosts where activities.host_idx=hosts.idx and hosts.host_name="'..host_ip..'") and contacts.contact_family='.. family..' and contacts.host_idx=hosts.idx group by contacts.contact_type,hosts.host_name order by tot desc,contacts.contact_type,hosts.host_name limit '..max_num
      --print(sql.."\n")
      for contact_type,host_name,num_contacts in db:urows(sql) do 
	 print(contact_type..",'"..host_name.."',"..num_contacts.."\n")
      end
   else
      sql = 'select contacts.contact_type,hosts.host_name,contacts.contact_family,contacts.num_contacts from contacts,hosts where contacts.activity_idx in (select activities.idx from activities,hosts where activities.host_idx=hosts.idx and hosts.host_name="'..host_ip..'") and contacts.host_idx=hosts.idx order by contacts.contact_type,hosts.host_name'
      
      for contact_type,host_name,contact_family,num_contacts in db:urows(sql) do 
	 print(contact_type..",'"..host_name.."',"..contact_family..","..num_contacts.."\n")
      end
   end
end

db:close()