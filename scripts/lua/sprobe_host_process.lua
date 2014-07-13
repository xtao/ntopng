--
-- (C) 2014 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

host_ip   = _GET["host"]
host_name = _GET["name"]
host_id   = _GET["id"]

if(mode ~= "embed") then
   sendHTTPHeader('text/html')
   ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")
   active_page = "hosts"
   dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")
end

print("<hr><h2><A HREF=/lua/host_details.lua?host="..host_ip..">"..host_name.."</A> Processes Interaction</H2>")

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/sprobe_process_header.inc")

print('d3.json("/lua/sprobe_host_process_data.lua?host='..host_ip..'&id='..host_id..'",')


print [[
      function(error, json) {
	    if (error) return console.warn(error);
	    links = json;

	    // Compute the distinct nodes from the links.
	    links.forEach(function(link) {
				if(isNaN(link.source)) {
				   /* IP Address -> PID */
				   _link = "/lua/sprobe_host_process.lua?host="+link.source+"&name="+link.source_name+"&id=0";
				} else {
				   /* PID -> IP Address */
				   _link = "/lua/get_process_info.lua?pid="+link.source+"&name="+link.source_name+"&host=]] print(host_ip) print [[&page=Flows";
				}
				link.source = nodes[link.source] || (nodes[link.source] = {name: link.source_name, num:link.source, link: _link, type: link.source_type, pid: link.source_pid });

				if(isNaN(link.target)) {
				   /* IP Address -> PID */
				   _link = "/lua/sprobe_host_process.lua?host="+link.target+"&name="+link.target_name+"&id=0";
				} else {
				   /* PID -> IP Address */
				   _link = "/lua/get_process_info.lua?pid="+link.target+"&name="+link.target_name+"&host=]] print(host_ip) print [[&page=Flows";
				}

				link.target = nodes[link.target] || (nodes[link.target] = {name: link.target_name, num: link.target, link: _link, type: link.target_type, pid: link.target_pid });
			     });

    ]]

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/sprobe_process.inc")

if(mode ~= "embed") then
dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
end
