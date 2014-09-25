print [[

 <style type='text/css'>
.largegroup {
    width:500px
}
</style>
<div id="password_dialog" class="modal fade" tabindex="-1" role="dialog" aria-labelledby="password_dialog_label" aria-hidden="true">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-header">
  <button type="button" class="close" data-dismiss="modal" aria-hidden="true">x</button>
  <h3 id="password_dialog_label">Manage User <span id="password_dialog_title"></span></h3>
</div>

<div class="modal-body">

  <div id="password_alert_placeholder"></div>

<script>
  password_alert = function() {}
  password_alert.error =   function(message) { $('#password_alert_placeholder').html('<div class="alert alert-danger"><button type="button" class="close" data-dismiss="alert">x</button>' + message + '</div>');
 }
  password_alert.success = function(message) { $('#password_alert_placeholder').html('<div class="alert alert-success"><button type="button" class="close" data-dismiss="alert">x</button>' + message + '</div>'); }
</script>

  <form id="form_password_reset" class="form-horizontal" method="get" action="password_reset.lua">
			   ]]
print('<input id="csrf" name="csrf" type="hidden" value="'..ntop.getRandomCSRFValue()..'" />\n')
print [[
    <input id="password_dialog_username" type="hidden" name="username" value="" />

<div class="input-group">
<div class="input-group">
<label for="" class="control-label">Old User Password</label>
<div class="input-group"><span class="input-group-addon"><i class="fa fa-lock"></i></span>  
  <input id="old_password_input" type="password" name="old_password" value="" class="form-control">
</div>
</div>

<div class="input-group">
  <label for="" class="control-label">New User Password</label>
<div class="input-group"><span class="input-group-addon"><i class="fa fa-lock"></i></span>  
  <input id="new_password_input" type="password" name="new_password" value="" class="form-control">
</div>
</div>

<div class="input-group">
  <label for="" class="control-label">Confirm New User Password</label>
<div class="input-group"><span class="input-group-addon"><i class="fa fa-lock"></i></span>  
  <input id="confirm_new_password_input" type="password" name="confirm_new_password" value="" class="form-control">
</div>
</div>

<div class="input-group">&nbsp;</div>
  <button id="password_reset_submit" class="btn btn-primary btn-block">Change User Password</button>
</div>

  </form>

]]

user_group = ntop.getUserGroup()
if(user_group["group"]=="administrator") then
print [[
<hr>
<form id="form_pref_change" class="form-horizontal" method="get" action="change_user_prefs.lua" role="form">
  <input id="pref_dialog_username" type="hidden" name="username" value="" />
<div class="input-group">

  <div class="input-group">
    <label class="input-label">User Role</label>
    <div class="controls">
      <select id="host_role_select" name="host_role">
                <option value="standard">Non Privileged User</option>
                <option value="administrator">Administrator</option>
      </select>
    </div>
  </div>


 <div class="largegroup input-group">
  <label for="" class="control-label">Allowed Networks</label>
<div class="input-group"><span class="input-group-addon"><span class="glyphicon glyphicon-tasks"></span></span>  
  <input id="allowed_networks_input" type="text" name="allowed_networks" value="" class="form-control" />
</div>
<small>Comma separated list of networks this user can view. Example: 192.168.1.0/24,172.16.0.0/16</small>
  </div>

<div class="input-group">&nbsp;</div> 
  <button id="pref_change" class="btn btn-primary btn-block">Change User Preferences</button>

</div>
  </form>
]]
end

print [[<script>
  var frmpassreset = $('#form_password_reset');
  frmpassreset.submit(function () {
    $.ajax({
      type: frmpassreset.attr('method'),
      url: frmpassreset.attr('action'),
      data: frmpassreset.serialize(),
      success: function (data) {
        var response = jQuery.parseJSON(data);
        if (response.result == 0)
          password_alert.success(response.message); 
        else
          password_alert.error(response.message);
        $("old_password_input").text("");
        $("new_password_input").text("");
        $("confirm_new_password_input").text("");
      }
    });
    return false;
  });

  var frmprefchange = $('#form_pref_change');
  frmprefchange.submit(function () {
    $.ajax({
      type: frmprefchange.attr('method'),
      url: frmprefchange.attr('action'),
      data: frmprefchange.serialize(),
      success: function (data) {
        var response = jQuery.parseJSON(data);
        if (response.result == 0)
          password_alert.success(response.message); 
        else
          password_alert.error(response.message);
      }
    });
    return false;
  });

</script>

</div> <!-- modal-body -->

<div class="modal-footer">
  <button class="btn btn-default btn-sm" data-dismiss="modal" aria-hidden="true">Close</button>
</div>

<script>
$('#password_reset_submit').click(function() {
  $('#form_password_reset').submit();
});
</script>
</div>
</div>
</div> <!-- password_dialog -->

			    ]]
