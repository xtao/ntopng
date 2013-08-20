<h4>Top Hosts (Send+Receive)</h4>
<div class="pie-chart" id="topHosts"></div>

<script type='text/javascript'>

window.onload=function() {
   var refresh = 3000 /* ms */;

   do_pie("#topHosts", '/lua/iface_hosts_list.lua', {  }, "", refresh);
}

</script>


