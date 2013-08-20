<h4>Top ASNs</h4>
<div class="pie-chart" id="topASN"></div>

<script type='text/javascript'>

window.onload=function() {
   var refresh = 3000 /* ms */;

   do_pie("#topASN", '/lua/top_asn.lua', {  }, "", refresh);
}

</script>
