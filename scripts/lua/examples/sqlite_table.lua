--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "sqlite_utils"

sendHTTPHeader('text/html')
ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")

dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

application = _GET["application"]
hosts = _GET["hosts"]
aggregation = _GET["aggregation"]
key = _GET["key"]

from = _GET["from"]
file = _GET["file"]


interface.find(ifname)
prefs = ntop.getPrefs()
ifstats = interface.getStats()

stats = interface.getNdpiStats()
num_param = 0

if (from ~= nil) then
  datetime_tbl = getParameters(from,file)
end

print [[
<div class="page-header">
        <h1>Historical Flows</h1>
      </div>
<div class="row well">
<form class="form-inline" role="form" method="get" onsubmit="return check_date()" action="/lua/examples/sqlite_table.lua">
  <div class='col-md-3 col-md-offset-1'>
    <div id='datetime_form' class="form-group">
      <label for="exampleInputEmail2">Date and Time</label>
      <div class='input-group date' id='datetime'>
          <input name="from" id='datetime_text' type='text' class="form-control" />
          <span class="input-group-addon"><span class="glyphicon glyphicon-calendar"></span>
          </span>
      </div>
    </div>
  </div>
  <div class='col-md-2 col-md-offset-1'>
    <br>
    <button type="submit" class="btn btn-default">Search</button>
    <button type="reset" class="btn btn-default">Reset</button>
  </div>
  <div class='col-md-3'>
    <ul class="pager">
      <li ><a id="older">&larr; Older</a></li>
      <li ><a id="newer">Newer &rarr;</a></li>
    </ul>
  </div>
  </div>
</form>

<form id="form_older" class="form-horizontal" method="get" action="/lua/examples/sqlite_table.lua">
  <input type="hidden" name="from" value="" id="file_from">
  <input type="hidden" name="file" value="" id="file_type">
</form>

<form id="form_newer" class="form-horizontal" method="get" action="/lua/examples/sqlite_table.lua">
  <input type="hidden" name="from" value="" id="file_from">
  <input type="hidden" name="file" value="" id="file_type">
</form>


        <script type="text/javascript">

         $("#older").click(function(){
            $("#file_from").val($('#datetime_text').val());
            $("#file_type").val("older");
            $("#form_older").submit();
        }); 

        $("#newer").click(function(){
            $("#file_from").val($('#datetime_text').val());
            $("#file_type").val("newer");
            $("#form_older").submit();
        }); 


        function addMinutes(date, minutes) {
          return new Date(date.getTime() + minutes*60000);
        }

        function removeMinutes(date, minutes) {
          return new Date(date.getTime() - minutes*60000);
        }

        $('#datetime').datetimepicker({ 
          minuteStepping:5,               //set the minute stepping
          language:'us', 
          pick12HourFormat: true]]

if ((from ~= nil) and (datetime_tbl ~= nil)) then
  print (',\ndefaultDate: "' .. datetime_tbl["displayed"].. '"')
end

print [[
        });

        function check_date () {

          var submit = true;

          console.log($('#datetime_text').val());

          var from = moment($('#datetime_text').val());
         
          console.log(from);
         
          if ((from > moment()) || (from.isValid() == false) ){
            $('#datetime_form').addClass("has-error has-feedback");
            submit = false;
          } else
            $('#datetime_form').addClass("has-success has-feedback");


          return submit;
        }
        </script>
      <div id="table-flows"></div>
   <script>
   var url_update = "/lua/get_flows_data.lua]]

   if(application ~= nil) then
   print("?application="..application)
   num_param = num_param + 1
end

if(hosts ~= nil) then
  if (num_param > 0) then
    print("&")
  else
    print("?")
  end
  print("hosts="..hosts)
  num_param = num_param + 1
end

if(aggregation ~= nil) then
  if (num_param > 0) then
    print("&")
  else
    print("?")
  end
  print("aggregation="..aggregation)
  num_param = num_param + 1
end

if(key ~= nil) then
  if (num_param > 0) then
    print("&")
  else
    print("?")
  end
  print("key="..key)
  num_param = num_param + 1
end

-- ACTIVE SQLITE 
if (num_param > 0) then
  print("&")
else
  print("?")
end
print("sqlite=")
ifname_id = interface.name2id(ifname)
if (datetime_tbl ~= nil) then
  query = "/" .. ifname_id .. "/flows"..datetime_tbl["query"] .. ".sqlite"
  print (query)
  -- io.write(query.."\n")
end

print ('";')

-- Set the flow table option
if(ifstats.iface_vlan) then print ('flow_rows_option["vlan"] = true;\n') end
   print [[

   var table = $("#table-flows").datatable({
      url: url_update , ]]

print [[ title: "",
         showFilter: true,
         showPagination: true,
]]

-- Automatic default sorted. NB: the column must be exists.
print ('sort: [ ["column_ID","asc"] ],');

print [[
         buttons: [ '<div class="btn-group"><button class="btn btn-link dropdown-toggle" data-toggle="dropdown">Applications<span class="caret"></span></button> <ul class="dropdown-menu" role="menu" id="flow_dropdown">]]

print('<li><a href="/lua/flows_stats.lua">All Proto</a></li>')
for key, value in pairsByKeys(stats["ndpi"], asc) do
   class_active = ''
   if(key == application) then
      class_active = ' class="active"'
   end
   print('<li '..class_active..'><a href="/lua/flows_stats.lua?application=' .. key..'">'..key..'</a></li>')
end

print("</ul> </div>' ],\n")

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/flows_stats_top.inc")

if(ifstats.iface_vlan) then
print [[
           {
           title: "VLAN",
         field: "column_vlan",
         sortable: true,
                 css: { 
              textAlign: 'center'
           }
         },
]]
end

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/flows_stats_middle.inc")


ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/flows_stats_bottom.inc")
dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
