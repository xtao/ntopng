require "os"

print [[ <hr>

      <div class="footer">


<div class="row-fluid show-grid">
	 <div class="span4">&copy; <A HREF="http://www.ntop.org">ntop.org</A> - 1998-]]

print(os.date("%Y"))
print [[ </div>
  <div class="span1 offset3"> <span class="network-load-chart">0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0</span>  </div>
  <div class="span2 offset2"><div id="network-load"></div> </div>

</div> <!-- /row --> 
</div><!-- /footer --> 


<script>
  // Updating charts.
var updatingChart = $(".network-load-chart").peity("line", { width: 64 })
var prev_bytes = 0;

function bytesToSize(bytes) {
      var sizes = ['Bytes', 'Kbps', 'Mbps', 'Gbps', 'Tbps'];
      if (bytes == 0) return 'n/a';
      var i = parseInt(Math.floor(Math.log(bytes) / Math.log(1024)));
      if (i == 0) return bytes + ' ' + sizes[i]; 
      return (bytes / Math.pow(1024, i)).toFixed(1) + ' ' + sizes[i];
   };

setInterval(function() {
		  $.ajax({
			    type: 'GET',
			    url: '/network_load.lua',
			    data: { if: "any" },
			    success: function(content) {
					   var rsp = jQuery.parseJSON(content);

					   if(prev_bytes > 0) {
					   var values = updatingChart.text().split(",")
					   var diff = rsp.bytes-prev_bytes;

					   values.shift()
					   values.push(diff)

					   updatingChart.text(values.join(",")).change()
					   $('#network-load').text(bytesToSize(diff*8));
					}
					   prev_bytes = rsp.bytes;
					}
				     });
			 }, 1000)

</script>

    </div> <!-- /container -->

  </body>
	 </html> ]]
