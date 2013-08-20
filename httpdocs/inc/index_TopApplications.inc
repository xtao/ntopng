<h4>Top Application Protocols</h4>
<div class="pie-chart" id="topApplicationProtocols"></div>

<script type='text/javascript'>

window.onload=function() {
   var refresh = 3000 /* ms */;

   do_pie("#topApplicationProtocols", '/lua/iface_ndpi_stats.lua', {  }, "", refresh);
}

</script>
