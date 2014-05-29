<div id="password_dialog" class="modal fade" tabindex="-1" role="dialog" aria-labelledby="password_dialog_label" aria-hidden="true">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-header">
  <button type="button" class="close" data-dismiss="modal" aria-hidden="true">x</button>
  <h3 id="password_dialog_label">Reset <span id="password_dialog_title"></span> Password</h3>
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
    <input id="password_dialog_username" type="hidden" name="username" value="" />

  <div class="control-group"> 
    <label class="control-label">Old Password</label>
    <div class="controls">
      <input id="old_password_input" type="password" name="old_password" value="" class="span4">
    </div>
  </div>

  <div class="control-group"> 
    <label class="control-label">New Password</label>
    <div class="controls">
      <input id="new_password_input" type="password" name="new_password" value="" class="span4">
    </div>
  </div>

  <div class="control-group"> 
    <label class="control-label">Confirm New Password</label>
    <div class="controls">
      <input id="confirm_new_password_input" type="password" name="confirm_new_password" value="" class="span4">
    </div>
  </div>

  </form>

<script>
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
</script>

</div> <!-- modal-body -->

<div class="modal-footer">
  <button class="btn btn-default btn-sm" data-dismiss="modal" aria-hidden="true">Close</button>
  <button id="password_reset_submit" class="btn btn-primary btn-sm">Change Password</button>
</div>

<script>
$('#password_reset_submit').click(function() {
  $('#form_password_reset').submit();
});
</script>
</div>
</div>
</div> <!-- password_dialog -->

