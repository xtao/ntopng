--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "graph_utils"

sendHTTPHeader('text/html')

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")
print("<link href=\"/css/tablesorted.css\" rel=\"stylesheet\">")
active_page = "if_stats"
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

page = _GET["page"]
if_name = _GET["if_name"]


if(if_name == nil) then if_name = ifname end

interface.find(if_name)
--print(if_name)
ifstats = interface.getStats()

if(_GET["custom_name"] ~=nil) then
      ntop.setCache('ntopng.prefs.'..ifstats.name..'.name',_GET["custom_name"])
end

rrdname = fixPath(dirs.workingdir .. "/" .. ifstats.id .. "/rrd/bytes.rrd")

if (if_name == nil) then 
  _ifname = ifname
else
  _ifname = if_name
end



url= '/lua/if_stats.lua?if_name=' .. _ifname



print [[
  <nav class="navbar navbar-default" role="navigation">
  <div class="navbar-collapse collapse">
    <ul class="nav navbar-nav">
]]

-- print("<li><a href=\"#\">Interface " .. ifname .."</a></li>\n")

if((page == "overview") or (page == nil)) then
  print("<li class=\"active\"><a href=\"#\">Overview</a></li>\n")
else
  print("<li><a href=\""..url.."&page=overview\">Overview</a></li>")
end

if(ifstats.type ~= "zmq") then
   if(page == "packets") then
      print("<li class=\"active\"><a href=\"#\">Packets</a></li>\n")
   else
      print("<li><a href=\""..url.."&page=packets\">Packets</a></li>")
   end
end

if(page == "ndpi") then
  print("<li class=\"active\"><a href=\"#\">Protocols</a></li>\n")
else
   print("<li><a href=\""..url.."&page=ndpi\">Protocols</a></li>")
end

if(ntop.exists(rrdname)) then
if(page == "historical") then
  print("<li class=\"active\"><a href=\"#\">Historical Activity</a></li>\n")
else
  print("<li><a href=\""..url.."&page=historical\">Historical Activity</a></li>")
end
end

print [[
</ul>
</div>
</nav>
   ]]
print ('<div id="alert_placeholder"></div>')
if((page == "overview") or (page == nil)) then
    
   print("<table class=\"table table-striped table-bordered\">\n")
   print("<tr><th width=250>Id</th><td colspan=2>" .. ifstats.id .. " ")
   print("</td></tr>\n")

   print("<tr><th width=250>State</th><td colspan=2>")
   state = toggleTableButton("", "", "Active", "1","primary", "Paused", "0","primary", "toggle_local", "ntopng.prefs."..if_name.."_not_idle")
   
   if(state == "0") then
      on_state = true
   else
      on_state = false
   end

   interface.setInterfaceIdleState(on_state)

   print("</td></tr>\n")

   print("<tr><th width=250>Name</th><td>" .. ifstats.name .. "</td>\n")
  
  if(ifstats.name ~= nil) then
    alternate_name = ntop.getCache('ntopng.prefs.'..ifstats.name..'.name')
    print [[
    <td>
    <form class="form-inline" style="margin-bottom: 0px;">
       <input type="hidden" name="if_name" value="]]
          print(ifstats.name)
    print [[">
       <input type="text" name="custom_name" placeholder="Custom Name" value="]]
          if(alternate_name ~= nil) then print(alternate_name) end
    print [["></input>
      &nbsp;<button type="submit" style="position: absolute; margin-top: 0; height: 26px" type="submit" class="btn btn-default btn-xs">Save Name</button>
    </form>
    </td></tr>
       ]]
  end
   
   print("<tr><th>Family</th><td colspan=2>" .. ifstats.type .. "</td></tr>\n")
   print("<tr><th>Bytes</th><td colspan=2><div id=if_bytes>" .. bytesToSize(ifstats.stats_bytes) .. "</div>");
   print [[
   <p>
   <small>
   <div class="alert alert-info">
      <b>NOTE</b>: In ethernet networks, each packet has an <A HREF=https://en.wikipedia.org/wiki/Ethernet_frame>overhead of 24 bytes</A> [preamble (7 bytes), start of freme (1 byte), CRC (4 bytes), and <A HREF=http://en.wikipedia.org/wiki/Interframe_gap>IFG</A> (12 bytes)]. Such overhead needs to be accounted to the interface traffic, but it is not added to the traffic being exchanged between IP addresses. This is because such data contributes to interface load, but it cannot be accounted in the traffic being exchanged by hosts, and thus expect little discrepancies between host and interface traffic values.
         </div></small>
   </td></tr>
   ]]

if(ifstats.type ~= "zmq") then
   label = "Packets"
else
   label = "Flows"
end
   print("<tr><th>Received Packets</th><td colspan=2><span id=if_pkts>" .. formatValue(ifstats.stats_packets) .. " " .. label .. "</span> <span id=pkts_trend></span></td></tr>\n")

   print("<tr><th>Dropped "..label.."</th><td colspan=2><span id=if_drops>")
   
   if(ifstats.stats_drops > 0) then print('<span class="label label-danger">') end
   print(formatValue(ifstats.stats_drops).. " " .. label)
   
   if((ifstats.stats_packets+ifstats.stats_drops) > 0) then
      local pctg = round((ifstats.stats_drops*100)/(ifstats.stats_packets+ifstats.stats_drops), 2)   
      if(pctg > 0) then print(" [ " .. pctg .. " % ] ") end
   end
   
   if(ifstats.stats_drops > 0) then print('</span>') end
   print("</span>  <span id=drops_trend></span></td></tr>\n")
   
   print("</table>\n")
elseif((page == "packets")) then
      print [[
      <table class="table table-bordered table-striped">
        <tr><th class="text-center">Size Distribution</th><td colspan=5><div class="pie-chart" id="sizeDistro"></div></td></tr>
      </table>

        <script type='text/javascript'>
         window.onload=function() {
       var refresh = 3000 /* ms */;
       do_pie("#sizeDistro", '/lua/if_pkt_distro.lua', { type: "size", ifname: "]] print(_ifname.."\"")
  print [[
           }, "", refresh);
    }

      </script><p>
  ]]
elseif(page == "ndpi") then
      print [[
      <script type="text/javascript" src="/js/jquery.tablesorter.js"></script>
      <table class="table table-bordered table-striped"> 
      <tr><th class="text-center">Protocol Overview</th><td colspan=5><div class="pie-chart" id="topApplicationProtocols"></div></td></tr>
  </div>

        <script type='text/javascript'>
         window.onload=function() {
       var refresh = 3000 /* ms */;
       do_pie("#topApplicationProtocols", '/lua/iface_ndpi_stats.lua', { mode: "sinceStartup", ifname: "]] print(_ifname) print [[" }, "", refresh);
    }

      </script><p>
  </table>
  ]]

  print [[
     <table id="myTable" class="table table-bordered table-striped tablesorter"> 
     ]]

     print("<thead><tr><th>Application Protocol</th><th>Total (Since Startup)</th><th>Percentage</th></tr></thead>\n")

  
  print ('<tbody id="if_stats_ndpi_tbody">\n')
  print ("</tbody>")
  print("</table>\n")
  print [[
<script>
function update_ndpi_table() {
  $.ajax({
    type: 'GET',
    url: '/lua/if_stats_ndpi.lua',
    data: { ifname: "]] print(tostring(interface.name2id(ifstats.name))) print [[" },
    success: function(content) { 
      $('#if_stats_ndpi_tbody').html(content);
    }
  });
}
update_ndpi_table();
setInterval(update_ndpi_table, 5000);
</script>

]]

else
   rrd_file = _GET["rrd_file"]
   if(rrd_file == nil) then rrd_file = "bytes.rrd" end
   drawRRD(ifstats.id, nil, rrd_file, _GET["graph_zoom"], url.."&page=historical", 1, _GET["epoch"], "/lua/top_talkers.lua")
end

dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")

print("<script>\n")
print("var last_pkts  = " .. ifstats.stats_packets .. ";\n")
print("var last_drops = " .. ifstats.stats_drops .. ";\n")


print [[
setInterval(function() {
      $.ajax({
          type: 'GET',
          url: '/lua/network_load.lua',
          data: { ifname: "]] print(tostring(interface.name2id(ifstats.name))) print [[" },
          success: function(content) {
        var rsp = jQuery.parseJSON(content);
        $('#if_bytes').html(bytesToVolume(rsp.bytes));
        $('#if_pkts').html(addCommas(rsp.packets)+"]]


if(ifstats.type == "zmq") then print(" Flows\");") else print(" Pkts\");") end
print [[
        var pctg = 0;
        var drops = "";

        if(last_pkts == rsp.packets) {
           $('#pkts_trend').html("<i class=\"fa fa-minus\"></i>");
        } else {
           $('#pkts_trend').html("<i class=\"fa fa-arrow-up\"></i>");
        }
        if(last_drops == rsp.drops) {
           $('#drops_trend').html("<i class=\"fa fa-minus\"></i>");
        } else {
           $('#drops_trend').html("<i class=\"fa fa-arrow-up\"></i>");
        }
        last_pkts = rsp.packets;
        last_drops = rsp.drops;

        if((rsp.packets+rsp.drops) > 0) { pctg = ((rsp.drops*100)/(rsp.packets+rsp.drops)).toFixed(2); }
        if(rsp.drops > 0) { drops = '<span class="label label-danger">'; }
        drops = drops + addCommas(rsp.drops)+" ]]

if(ifstats.type == "zmq") then print("Flows") else print("Pkts") end
print [[";

        if(pctg > 0)      { drops = drops + " [ "+pctg+" % ]"; }
        if(rsp.drops > 0) { drops = drops + '</span>';         }
        $('#if_drops').html(drops);
           }
               });
       }, 3000)

</script>

]]
print [[
<script type="text/javascript" src="/js/jquery.tablesorter.js"></script>
<script>
$(document).ready(function() 
    { 
        $("#myTable").tablesorter(); 
    } 
); 
</script>
]]