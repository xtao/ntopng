--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path
if ( (dirs.scriptdir ~= nil) and (dirs.scriptdir ~= "")) then package.path = dirs.scriptdir .. "/lua/modules/?.lua;" .. package.path end

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
is_historical = interface.isHistoricalInterface(interface.name2id(ifname))
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

short_name = string.sub(ifname, 1, 30)
if(short_name ~= ifname) then
   short_name = short_name .. "..."
end

print("<li><a href=\"#\">Interface: " .. short_name .."</a></li>\n")

if((page == "overview") or (page == nil)) then
  print("<li class=\"active\"><a href=\"#\">Overview</a></li>\n")
else
  print("<li><a href=\""..url.."&page=overview\">Overview</a></li>")
end

-- Disable Packets and Protocols tab in case of the number of packets is equal to 0
if((ifstats ~= nil) and (ifstats.stats_packets > 0)) then
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
end

if (is_historical) then
  if(page == "config_historical") then
    print("<li class=\"active\"><a href=\"#\">Load Data</a></li>\n")
  else
    print("<li><a href=\""..url.."&page=config_historical\">Load Data</a></li>")
  end

else

  if(ntop.exists(rrdname)) then
    if(page == "historical") then
      print("<li class=\"active\"><a href=\"#\">Historical Activity</a></li>\n")
    else
      print("<li><a href=\""..url.."&page=historical\">Historical Activity</a></li>")
    end
  end

end

print [[
</ul>
</div>
</nav>
   ]]

if((page == "overview") or (page == nil)) then
   print("<table class=\"table table-striped table-bordered\">\n")
   print("<tr><th width=250>Id</th><td colspan=2>" .. ifstats.id .. " ")
   print("</td></tr>\n")
  if not (is_historical) then
   print("<tr><th width=250>State</th><td colspan=2>")
   state = toggleTableButton("", "", "Active", "1","primary", "Paused", "0","primary", "toggle_local", "ntopng.prefs."..if_name.."_not_idle")

   if(state == "0") then
      on_state = true
   else
      on_state = false
   end

   interface.setInterfaceIdleState(on_state)

   print("</td></tr>\n")
  end
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

   if(ifstats.name ~= ifstats.description) then
      print("<tr><th>Description</th><td colspan=2>" .. ifstats.description .. "</td></tr>\n")
   end

   print("<tr><th>Family</th><td colspan=2>" .. ifstats.type .. "</td></tr>\n")
   print("<tr><th>Bytes</th><td colspan=2><div id=if_bytes>" .. bytesToSize(ifstats.stats_bytes) .. "</div>");
   print [[
   <p>
   <small>
   <div class="alert alert-info">
      <b>NOTE</b>: In ethernet networks, each packet has an <A HREF=https://en.wikipedia.org/wiki/Ethernet_frame>overhead of 24 bytes</A> [preamble (7 bytes), start of frame (1 byte), CRC (4 bytes), and <A HREF=http://en.wikipedia.org/wiki/Interframe_gap>IFG</A> (12 bytes)]. Such overhead needs to be accounted to the interface traffic, but it is not added to the traffic being exchanged between IP addresses. This is because such data contributes to interface load, but it cannot be accounted in the traffic being exchanged by hosts, and thus expect little discrepancies between host and interface traffic values.
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

elseif(page == "historical") then
   rrd_file = _GET["rrd_file"]
   if(rrd_file == nil) then rrd_file = "bytes.rrd" end
   drawRRD(ifstats.id, nil, rrd_file, _GET["graph_zoom"], url.."&page=historical", 1, _GET["epoch"], "/lua/top_talkers.lua")



--
--  Historical Interface configuration page
--
elseif(page == "config_historical") then

  historical_info = interface.getHistorical()

  print ('<div id="alert_placeholder"></div>')

   print('<form class="form-horizontal" role="form" method="get" id="conf_historical_form" action="/lua/config_historical_intreface.lua">')
   print[[
    <input type="hidden" name="from" value="" id="form_from">
    <input type="hidden" name="to" value="" id="form_to">
    <input type="hidden" name="id" value="" id="form_interface_id">
   ]]
   print("<table class=\"table table-striped table-bordered\">\n")
   print("<tr><th >Begin Date/Time</th><td colspan=2>")
   print [[
   <div class='input-group date' id='datetime_from'>
          <span class="input-group-addon"><span class="glyphicon glyphicon-calendar"></span></span>
          <input id='datetime_from_val' type='text' class="form-control" readonly/>
    </div>
    <span class="help-block">Specify the date and time from which to begin loading data.</span>
   ]]
   print("</td></tr>\n")

   print("<tr><th >End Date/Time</th><td colspan=2>")
   print [[
   <div class='input-group date' id='datetime_to'>
          <span class="input-group-addon"><span class="glyphicon glyphicon-calendar"></span></span>
          <input id='datetime_to_val' type='text' class="form-control" readonly/>
    </div>
    <span class="help-block">Specify the end of the loading interval.</span>
   ]]
   print("</td></tr>\n")


print("<tr><th >Source Interface</th><td colspan=2>")
   print [[
   <div class="btn-group">
    ]]

names = interface.getIfNames()

 key = 'ntopng.prefs.'..historical_info["interface_name"]..'.name'
custom_name = ntop.getCache(key)
current_name = historical_info["interface_name"]

if((custom_name ~= nil) and (custom_name ~= "")) then
  current_name = custom_name
end


print('<button id="interface_displayed"  value="' .. historical_info["interface_name"].. '" class="btn btn-default btn-sm dropdown-toggle" data-toggle="dropdown">' .. current_name.. '<span class="caret"></span></button>\n')

print('    <ul class="dropdown-menu" id="interface_list">\n')

for k,v in pairs(names) do
  key = 'ntopng.prefs.'..v..'.name'
  custom_name = ntop.getCache(key)
    if (v ~= "Historical") then
      print('<li><a name="' ..v..'" >')
      if((custom_name ~= nil) and (custom_name ~= "")) then
        print(custom_name..'</a></li>')
      else
        print(v..'</a></li>')
      end
    end
end

print [[
            </ul>
          </div><!-- /btn-group -->
          <span class="help-block">Specify the interface from which to load the data (previously saved into your data directory).</span>
   ]]
   print("</td></tr>\n")

print [[
<tr><th colspan=3 class="text-center">
      <button type="submit" class="btn btn-default">Load Historical Data</button>
      <button type="reset" class="btn btn-default">Reset Form</button>
</th></tr>
</table>
</form>
]]

print [[
<form id="start_historical" class="form-horizontal" method="get" action="/lua/config_historical_intreface.lua">
  <input type="hidden" name="from" value="" id="form_from">
  <input type="hidden" name="to" value="" id="form_to">
  <input type="hidden" name="id" value="" id="form_interface_id">
</form>
]]

actual_time =os.time()
mod = actual_time%300
actual_time = actual_time - mod

print [[
<script>

$('#interface_list li > a').click(function(e){
    $('#interface_displayed').html(this.innerHTML+' <span class="caret"></span>');
    $('#interface_displayed').val(this.name);
  });

$('#datetime_from').datetimepicker({
          minuteStepping:5,               //set the minute stepping
          language:'en',
          pick12HourFormat: false]]

if ((historical_info["from_epoch"] ~= nil) and (historical_info["from_epoch"] ~= 0) )then
   print (',\ndefaultDate: moment('..tonumber(historical_info["from_epoch"]*1000)..')')
else
  print (',\ndefaultDate: moment('..tonumber(actual_time - 600) * 1000 ..')')
end
print (',\nmaxDate: "'.. os.date("%x", actual_time+ 86400) .. '"') -- One day more in order to enable today (library issue)

print [[
        });

$('#datetime_to').datetimepicker({
          minuteStepping:5,               //set the minute stepping
          language:'en',
          pick12HourFormat: false]]

if ((historical_info["to_epoch"] ~= nil) and (historical_info["to_epoch"] ~= 0) )then
   print (',\ndefaultDate: moment('..tonumber(historical_info["to_epoch"]*1000)..')')
else
  print (',\ndefaultDate: moment('..tonumber(actual_time - 300) * 1000 ..')')
end

print (',\nmaxDate: "'.. os.date("%x", actual_time+ 86400) .. '"') -- One day more in order to enable today (library issue)

print [[
        });

  function check_date () {

    var submit = true;
    var from = $('#datetime_from_val').val();
    var to = $('#datetime_to_val').val();

    if (from == "" || from == NaN) {
       $('#datetime_from').addClass("has-error has-feedback");
       $('#alert_placeholder').html('<div class="alert alert-warning"><button type="button" class="close" data-dismiss="alert">x</button><strong> Invalid From:</strong> please select form date and time.</div>');
      return false;
    }

    if (to == ""|| to == NaN) {
       $('#datetime_to').addClass("has-error has-feedback");
      return false;
    }

    var from_epoch = moment(from);
    var from_unix = from_epoch.unix();
    var to_epoch = moment(to);
    var to_unix = to_epoch.unix();

    if ((from_epoch > moment()) || (from_epoch.isValid() == false) ){
      $('#datetime_from').addClass("has-error has-feedback");
      $('#alert_placeholder').html('<div class="alert alert-warning"><button type="button" class="close" data-dismiss="alert">x</button><strong> Invalid From:</strong> please choose a valid date and time.</div>');
      submit = false;
    } else {
      $('#datetime_from').addClass("has-success has-feedback");
    }

    if ((to_epoch > moment()) || (to_epoch.isValid() == false) ){
      $('#datetime_to').addClass("has-error has-feedback");
       $('#alert_placeholder').html('<div class="alert alert-warning"><button type="button" class="close" data-dismiss="alert">x</button><strong> Invalid To:</strong> please choose a valid date and time.</div>');
      submit = false;
    } else {
      $('#datetime_to').addClass("has-success has-feedback");
    }

    $('#form_from').val( from_unix);
    $('#form_to').val(to_unix );
    $('#form_interface_id').val($('#interface_displayed').val());

    return submit;

  }


$( "#conf_historical_form" ).submit(function( event ) {
  var frm = $('#conf_historical_form');
  $('#alert_placeholder').html("");

  if (check_date()) {
    $.ajax({
      type: frm.attr('method'),
      url: frm.attr('action'),
      data: frm.serialize(),
      async: false,
      success: function (data) {
        var response = jQuery.parseJSON(data);
        if (response.result == "0") {
            $('#alert_placeholder').html('<div class="alert alert-success"><button type="button" class="close" data-dismiss="alert">x</button><strong>Well Done!</strong> Data loading process started successfully</div>');
        } else {
          $('#alert_placeholder').html('<div class="alert alert-warning"><button type="button" class="close" data-dismiss="alert">x</button><strong>Warning</strong> Please wait that the loading process will be complete.<br></div>');
        }
      }
    });
   //window.setTimeout('window.location="index.lua"; ', 3000);
  }
  event.preventDefault();
});

</script>

 ]]
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
