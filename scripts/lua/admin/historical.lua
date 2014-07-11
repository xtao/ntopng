--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"


interface_id = _GET["id"]
from = _GET["from"]
to = _GET["to"]
sendHTTPHeader('text/html')


ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")

active_page = "admin"
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")


if ((from ~= nil) and (to ~= nil) and (interface_id ~= nil)) then

  ntop.startHistoricalInterface(tonumber(from), tonumber(to), interface.name2id(interface_id))

  print [[
    <br>
    <div class="alert alert-success">
      <button type="button" class="close" data-dismiss="alert" aria-hidden="true">&times;</button>
      <strong>Well Done</strong> Historical interface configured and active.
    </div>

    <script>
    window.setTimeout('window.location="../index.lua"; ', 3000);
  </script>

    ]]



else

historical_info = ntop.getHistorical()
-- if (historical_info ~= nil) then
--   print(historical_info["id"] .. '</br>')
--   print(historical_info["interface_id"] .. '</br>')
--     print(historical_info["interface_name"] .. '</br>')
--   print(historical_info["from_epoch"] .. '</br>')
--   print(historical_info["to_epoch"] .. '</br>')
--   if (historical_info["running"]) then print( 'running</br>') else print('stop </br>') end
-- end


print [[
</br>
<div class="container">

  <form class="form-horizontal col-lg-5" role="form" method="get" onsubmit="return check_date()" action="">
   <legend>Configure Historical Interface</legend> </br>
  <div class="form-group">
    <label class="col-sm-3">From </label>
    <div class='input-group date' id='datetime_from'>
          <span class="input-group-addon"><span class="glyphicon glyphicon-calendar"></span></span>
          <input name="from" id='datetime_from_val' type='text' class="form-control" readonly/>
    </div>
  </div>

  <div class="form-group">
    <label class="col-sm-3">To </label>
    <div class='input-group date' id='datetime_to'>
          <span class="input-group-addon"><span class="glyphicon glyphicon-calendar"></span></span>
          <input name="to" id='datetime_to_val' type='text' class="form-control" readonly/>
    </div>
  </div>

  <div class="form-group">
    <label class="col-sm-3">Interface </label>
    <input type="hidden" name="id" value="" id="interface_id">
    <div class="btn-group">
    ]]



names = interface.getIfNames()

print('<button id="interface_displayed"  value="' .. historical_info["interface_name"] ..'" class="btn btn-default btn-sm dropdown-toggle" data-toggle="dropdown">'..historical_info["interface_name"]..'<span class="caret"></span></button>\n')

print('    <ul class="dropdown-menu" id="interface_list">\n')

for k,v in pairs(names) do
    if (v ~= "Historical") then
      print('<li><a>'..v..'</a></li>')
    end
end

print [[
            </ul>
          </div><!-- /btn-group -->
  </div>

<br>
  <div class="form-group">
    <div class="col-sm-offset-2 col-sm-10">
      <button type="submit" class="btn btn-default">Load</button>
      <button type="reset" class="btn btn-default">Reset</button>
    </div>
  </div>
</form>
</div>
]]


print [[
<form id="start_historical" class="form-horizontal" method="get" action="">
  <input type="hidden" name="from" value="" id="form_from">
  <input type="hidden" name="to" value="" id="form_to">
  <input type="hidden" name="id" value="" id="form_interface_id">
</form>
]]

print [[
<script>

$('#interface_list li > a').click(function(e){
    $('#interface_displayed').html(this.innerHTML+' <span class="caret"></span>');
    $('#interface_displayed').val(this.innerHTML);
  });

$('#datetime_from').datetimepicker({
          minuteStepping:5,               //set the minute stepping
          language:'us',
          pick12HourFormat: true]]

if ((historical_info["from_epoch"] ~= nil) and (historical_info["from_epoch"] ~= 0) )then
   print (',\ndefaultDate: moment('..tonumber(historical_info["from_epoch"]*1000)..')')
end

print [[
        });


$('#datetime_to').datetimepicker({
          minuteStepping:5,               //set the minute stepping
          language:'us',
          pick12HourFormat: true]]

if ((historical_info["to_epoch"] ~= nil) and (historical_info["to_epoch"] ~= 0) )then
   print (',\ndefaultDate: moment('..tonumber(historical_info["to_epoch"]*1000)..')')
end

print [[
        });


  function check_date () {

          var submit = true;


          //alert($('#interface_displayed').val());

          //console.log('-' +$('#datetime_from_val').val() + '-');
          //console.log($('#datetime_to_val').val());

          var from = $('#datetime_from_val').val();
          var to = $('#datetime_to_val').val();

          if (from == "" || from == NaN) {
             $('#datetime_from').addClass("has-error has-feedback");
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

          //console.log(from_epoch);
          //console.log(to_epoch);

          if ((from_epoch > moment()) || (from_epoch.isValid() == false) ){
            $('#datetime_from').addClass("has-error has-feedback");
            submit = false;
          } else {
            $('#datetime_from').addClass("has-success has-feedback");
          }


          if ((to_epoch > moment()) || (to_epoch.isValid() == false) ){
            $('#datetime_to').addClass("has-error has-feedback");
            submit = false;
          } else {
            $('#datetime_to').addClass("has-success has-feedback");
          }


          $('#form_from').val( from_unix);
          $('#form_to').val(to_unix );
          $('#form_interface_id').val($('#interface_displayed').val());

          if (submit ) $('#start_historical').submit();


          return false;
        }

</script>
]]

end

dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
