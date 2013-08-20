            <h4>Top Flow Senders: Live</h4>
	    <div class="pie-chart" id="topSenders"></div>

<script type='text/javascript'>

window.onload=function() {
   var refresh = 3000 /* ms */;

   do_pie("#topSenders", '/lua/top_talkers.lua', { mode : "senders" }, "", refresh);
}

</script>
