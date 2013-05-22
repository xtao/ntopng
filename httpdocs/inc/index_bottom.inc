
      </div>

      <div class="row-fluid marketing">
        <div class="span6">
          <h4>Top Hosts</h4>
	  <div class="pie-chart" id="topHosts"></div>
        </div>
        <div class="span6">
          <h4>Top Application Protocols</h4>
	  <div class="pie-chart" id="topApplicationProtocols"></div>
        </div>

      </div>


<script type='text/javascript'>

window.onload=function() {
   var refresh = 3000 /* ms */;

   do_pie("#topHosts", '/iface_hosts_list.lua', { if: "any" }, "", refresh);
   do_pie("#topApplicationProtocols", '/iface_ndpi_stats.lua', { if: "any" }, "", refresh);
}

</script>
