
<div id="add_user_dialog" class="modal fade" tabindex="-1" role="dialog" aria-labelledby="add_user_dialog_label" aria-hidden="true">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-header">
  <button type="button" class="close" data-dismiss="modal" aria-hidden="true">x</button>
  <h3 id="add_user_dialog_label">Add User</h3>
</div>

<div class="modal-body">

  <div id="add_user_alert_placeholder"></div>

<script>
  add_user_alert = function() {}
  add_user_alert.error =   function(message) { $('#add_user_alert_placeholder').html('<div class="alert alert-danger"><button type="button" class="close" data-dismiss="alert">x</button>' + message + '</div>');
 }
  add_user_alert.success = function(message) { $('#add_user_alert_placeholder').html('<div class="alert alert-success"><button type="button" class="close" data-dismiss="alert">x</button>' + message + '</div>'); }
</script>

  <form id="form_add_user" class="form-horizontal" method="get" action="add_user.lua">

  <div class="control-group"> 
    <label class="control-label">Username</label>
    <div class="controls">
      <input id="username_input" type="text" name="username" value="" class="span4">
    </div>
  </div>

  <div class="control-group"> 
    <label class="control-label">Full Name</label>
    <div class="controls">
      <input id="full_name_input" type="text" name="full_name" value="" class="span4">
    </div>
  </div>

  <div class="control-group"> 
    <label class="control-label">Password</label>
    <div class="controls">
      <input id="password_input" type="password" name="password" value="" class="span4">
    </div>
  </div>

  <div class="control-group"> 
    <label class="control-label">Confirm Password</label>
    <div class="controls">
      <input id="confirm_password_input" type="password" name="confirm_password" value="" class="span4">
    </div>
  </div>

  </form>

<script>
  var frmadduser = $('#form_add_user');
  frmadduser.submit(function () {
    $.ajax({
      type: frmadduser.attr('method'),
      url: frmadduser.attr('action'),
      data: frmadduser.serialize(),
      success: function (data) {
        var response = jQuery.parseJSON(data);
        if (response.result == 0) {
          add_user_alert.success(response.message); 
          window.location.href = 'users.lua';
        } else {
          add_user_alert.error(response.message);
        }
        frmadduser[0].reset();
      }
    });
    return false;
  });
</script>

</div> <!-- modal-body -->

<div class="modal-footer">
  <button class="btn btn-default btn-sm" data-dismiss="modal" aria-hidden="true">Close</button>
  <button id="add_user_submit" class="btn btn-primary btn-sm">Add</button>
</div>

<script>
$('#add_user_submit').click(function() {
  $('#form_add_user').submit();
});
</script>

</div>
</div>
</div> <!-- add_user_dialog -->

