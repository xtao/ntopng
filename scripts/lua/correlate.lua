--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "graph_utils"

host_ip     = _GET["host"]

active_page = "hosts"

sendHTTPHeader('text/html')
ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

if(host_ip == nil) then
   print("<div class=\"alert alert-error\"><img src=/img/warning.png> Host parameter is missing (internal error ?)</div>")
   return
end

if(protocol_id == nil) then protocol_id = "" end

_ifname = tostring(interface.name2id(ifname))
interface.find(ifname)

pearson = interface.correlateHostActivity(host_ip)

print("<H2>Host: <A HREF=/lua/host_details.lua?host="..host_ip..">"..host_ip.."</A></H2><p>\n")
print("<table border=1>\n")

vals = {}
for k,v in pairs(pearson) do
   vals[v] = k
end

max_hosts = 10

n = 0
for v,k in pairsByKeys(vals, rev) do
   print("<tr><th align=left><A HREF=/lua/host_details.lua?host="..k..">"..k.."</a></th><td>"..v.."</td></tr>\n")
   n = n +1

   if(n >= max_hosts) then
      break
   end
end
print("</table>\n")

dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
