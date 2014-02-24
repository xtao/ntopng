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

--for k,v in pairs(host["dns"][what]) do
--   print(k.."="..v.."<br>\n")
--end

if(host ~= nil) then
   tot = host["dns"][what]["num_a"] + host["dns"][what]["num_ns"] + host["dns"][what]["num_cname"] + host["dns"][what]["num_soa"] + host["dns"][what]["num_ptr"] + host["dns"][what]["num_mx"]  + host["dns"][what]["num_txt"] + host["dns"][what]["num_aaaa"] + host["dns"][what]["num_any"]
   
   if(tot > 0) then
      min = (tot * 3)/100
      comma = ""

      if(host["dns"][what]["num_a"] > min) then 
	 print('\t { "label": "A", "value": '.. host["dns"][what]["num_a"] .. '}\n')
	 comma = "," 
      else 
	 left = left + host["dns"][what]["num_a"]
      end

      if(host["dns"][what]["num_ns"] > min) then
	 print(comma..'\t { "label": "NS", "value": '.. host["dns"][what]["num_ns"] .. '}\n')
	 comma = "," 
      else
	 left = left + host["dns"][what]["num_ns"]
      end

      if(host["dns"][what]["num_cname"] > min) then
	 print(comma..'\t { "label": "CNAME", "value": '.. host["dns"][what]["num_cname"] .. '}\n') 
	 comma = "," 
      else
	 left = left + host["dns"][what]["num_cname"] 
      end

      if(host["dns"][what]["num_soa"] > min) then
	 print(comma..'\t { "label": "SOA", "value": '.. host["dns"][what]["num_soa"] .. '}\n') 
	 comma = "," 
      else
	 left = left + host["dns"][what]["num_soa"] 
      end

      if(host["dns"][what]["num_ptr"] > min) then
	 print(comma..'\t { "label": "PTR", "value": '.. host["dns"][what]["num_ptr"] .. '}\n') 
	 comma = "," 
      else
	 left = left + host["dns"][what]["num_ptr"]
      end

      if(host["dns"][what]["num_mx"] > min) then
	 print(comma..'\t { "label": "MX", "value": '.. host["dns"][what]["num_mx"] .. '}\n') 
	 comma = "," 
      else
	 left = left + host["dns"][what]["num_mx"] 
      end

      if(host["dns"][what]["num_txt"] > min) then
	 print(comma..'\t { "label": "TXT", "value": '.. host["dns"][what]["num_txt"] .. '}\n')
	 comma = "," 
      else
	 left = left + host["dns"][what]["num_txt"]
      end

      if(host["dns"][what]["num_aaaa"] > min) then
	 print(comma..'\t { "label": "AAAA", "value": '.. host["dns"][what]["num_aaaa"] .. '}\n') 
	 comma = "," 
      else
	 left = left + host["dns"][what]["num_aaaa"]
      end

      if(host["dns"][what]["num_any"] > min) then
	 print(comma..'\t { "label": "ANY", "value": '.. host["dns"][what]["num_any"] .. '}\n') 
	 comma = "," 
      else
	 left = left + host["dns"][what]["num_any"] 
      end
      
      other = host["dns"][what]["num_other"] + left
      if(other > 0) then print(comma..'\t { "label": "Other", "value": '.. other .. '}\n') 
      end
   end
end
	 
print "\n]"




