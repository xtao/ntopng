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

left = 0

print "[\n"

if(host ~= nil) then
   min = (host["dns"][what]["num_queries"] * 1)/100
   if(host["dns"][what]["num_a"] > min) then print('\t { "label": "A", "value": '.. host["dns"][what]["num_a"] .. '},\n') else left = left + host["dns"][what]["num_a"] end
   if(host["dns"][what]["num_ns"] > min) then print('\t { "label": "NS", "value": '.. host["dns"][what]["num_ns"] .. '},\n') else left = left + host["dns"][what]["num_ns"] end
   if(host["dns"][what]["num_cname"] > min) then print('\t { "label": "CNAME", "value": '.. host["dns"][what]["num_cname"] .. '},\n') else left = left + host["dns"][what]["num_cname"] end
   if(host["dns"][what]["num_soa"] > min) then print('\t { "label": "SOA", "value": '.. host["dns"][what]["num_soa"] .. '},\n') else left = left + host["dns"][what]["num_soa"] end
   if(host["dns"][what]["num_ptr"] > min) then print('\t { "label": "PTR", "value": '.. host["dns"][what]["num_ptr"] .. '},\n') else left = left + host["dns"][what]["num_ptr"] end
   if(host["dns"][what]["num_mx"] > min) then print('\t { "label": "MX", "value": '.. host["dns"][what]["num_mx"] .. '},\n') else left = left + host["dns"][what]["num_mx"] end
   if(host["dns"][what]["num_txt"] > min) then print('\t { "label": "TXT", "value": '.. host["dns"][what]["num_txt"] .. '},\n') else left = left + host["dns"][what]["num_txt"] end
   if(host["dns"][what]["num_aaaa"] > min) then print('\t { "label": "AAAA", "value": '.. host["dns"][what]["num_aaaa"] .. '},\n') else left = left + host["dns"][what]["num_aaaa"] end
   if(host["dns"][what]["num_any"] > min) then print('\t { "label": "ANY", "value": '.. host["dns"][what]["num_any"] .. '},\n') else left = left + host["dns"][what]["num_any"] end

   other = host["dns"][what]["num_other"] + left
   if(other > 0) then print('\t { "label": "Other", "value": '.. other .. '}\n') end
end
	 
print "\n]"




