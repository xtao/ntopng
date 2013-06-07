
      </div>

      <div class="row-fluid marketing">
        <div class="span6">
          <div class="pie-chart-box">
            <h4>Top Hosts (Send+Receive)</h4>
	    <div class="pie-chart" id="topHosts"></div>
          </div>
        </div>
        <div class="span6">
          <div class="pie-chart-box">
            <h4>Top Application Protocols</h4>
	    <div class="pie-chart" id="topApplicationProtocols" style="margin-left:auto;margin-right:auto"></div>
          </div>
        </div>
      </div>

      <div class="row-fluid marketing">
        <div class="span6">
          <div class="pie-chart-box">
            <h4>Top ASNs</h4>
	    <div class="pie-chart" id="topASN"></div>
          </div>
        </div>
        <div class="span6">
          <div class="pie-chart-box">
            <h4>Top Flow Senders: Live</h4>
	    <div class="pie-chart" id="topSenders"></div>
          </div>
        </div>
      </div>


<script type='text/javascript'>

window.onload=function() {
   var refresh = 3000 /* ms */;

   do_pie("#topHosts", '/iface_hosts_list.lua', { if: "any" }, "", refresh);
   do_pie("#topApplicationProtocols", '/iface_ndpi_stats.lua', { if: "any" }, "", refresh);
   do_pie("#topASN", '/top_asn.lua', { if : "any" }, "", refresh);
   do_pie("#topSenders", '/top_talkers.lua', { mode : "senders" }, "", refresh);
}

</script>
