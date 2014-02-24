--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')

interface.find(ifname)

host_ip = _GET["host"]
mode = _GET["mode"]

if(mode == "sent") then
   what = "sent"
else
   what = "rcvd"
end

host = interface.getHostInfo(host_ip)

print "[\n"

if(host ~= nil) then
   print('\t { "label": "A", "value": '.. host["dns"][what]["num_a"] .. '},\n')
   print('\t { "label": "NS", "value": '.. host["dns"][what]["num_ns"] .. '},\n')
   print('\t { "label": "CNAME", "value": '.. host["dns"][what]["num_cname"] .. '},\n')
   print('\t { "label": "SOA", "value": '.. host["dns"][what]["num_soa"] .. '},\n')
   print('\t { "label": "PTR", "value": '.. host["dns"][what]["num_ptr"] .. '},\n')
   print('\t { "label": "MX", "value": '.. host["dns"][what]["num_mx"] .. '},\n')
   print('\t { "label": "TXT", "value": '.. host["dns"][what]["num_txt"] .. '},\n')
   print('\t { "label": "AAAA", "value": '.. host["dns"][what]["num_aaaa"] .. '},\n')
   print('\t { "label": "ANY", "value": '.. host["dns"][what]["num_any"] .. '},\n')
   print('\t { "label": "Other", "value": '.. host["dns"][what]["num_other"] .. '}\n')
end

print "\n]"




