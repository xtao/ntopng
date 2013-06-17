
<div id="password_dialog" class="modal hide fade" tabindex="-1" role="dialog" aria-labelledby="password_dialog_label" aria-hidden="true">

<div class="modal-header">
  <button type="button" class="close" data-dismiss="modal" aria-hidden="true">x</button>
  <h3 id="password_dialog_label">Reset <span id="password_dialog_title"></span> Password</h3>
</div>

<div class="modal-body">

  <div id="alert_placeholder"></div>

<script>
  bootstrap_alert = function() {}
  bootstrap_alert.error =   function(message) { $('#alert_placeholder').html('<div class="alert alert-error"><button type="button" class="close" data-dismiss="alert">x</button>' + message + '</div>');
 }
  bootstrap_alert.success = function(message) { $('#alert_placeholder').html('<div class="alert alert-success"><button type="button" class="close" data-dismiss="alert">x</button>' + message + '</div>'); }
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
  var frm = $('#form_password_reset');
  frm.submit(function () {
    $.ajax({
      type: frm.attr('method'),
      url: frm.attr('action'),
      data: frm.serialize(),
      success: function (data) {
        var response = jQuery.parseJSON(data);
        if (response.result == 0)
          bootstrap_alert.success(response.message); 
        else
          bootstrap_alert.error(response.message);
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
  <button class="btn" data-dismiss="modal" aria-hidden="true">Close</button>
  <button id="password_reset_submit" class="btn btn-primary">Change Password</button>
</div>

<script>
$('#password_reset_submit').click(function() {
  $('#form_password_reset').submit();
});
</script>

</div> <!-- password_dialog -->

