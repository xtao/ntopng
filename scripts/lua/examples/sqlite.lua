--
-- (C) 2014 - ntop.org
--

-- Hello world

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')


ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")


dirs = ntop.getDirs()
rsp = ntop.execQuery(dirs.workingdir .. "/0/flows/14/05/31/12/45.sqlite", "SELECT * from flows LIMIT 10")

if(rsp == nil) then
   print("Query error")
else
   print("<table class=\"table table-bordered table-striped\">\n")
   
   num = 0
   for _k,_v in pairs(rsp) do
      
      if(num == 0) then
	 print("<tr><th>Id</th>")
	 for k,v in pairs(_v) do
	    print("<th>".. k .."</th>")
	 end
	 print("</tr>\n")
      end
      
      print("<tr>")
      print("<th>".. num .."</th>")
      
      for k,v in pairs(_v) do
	 print("<td>".. v .."</td>")
      end
      
      print("</tr>\n")
      num = num + 1
   end

   print("</table>\n")
end

dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")