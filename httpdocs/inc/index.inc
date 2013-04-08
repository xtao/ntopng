
      <div class="masthead">
        <ul class="nav nav-pills pull-right">
          <li class="active"><a href="#">Home</a></li>
          <li><a href="/interface_stats.lua">Interfaces</a></li>
       	     <li><a href="/hosts_stats.lua">Hosts</a></li>
        </ul>
        <h3 class="muted"><img src="/img/logo.png"></h3>
      </div>

      <hr>
      <div class="jumbotron">
        <h1>Super awesome marketing speak!</h1>
        <p class="lead">Cras justo odio, dapibus ac facilisis in, egestas eget quam. Fusce dapibus, tellus ac cursus commodo, tortor mauris condimentum nibh, ut fermentum massa justo sit amet risus.</p>
        <a class="btn btn-large btn-success" href="#">Sign up today</a>
      </div>

      <hr>

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

   do_pie("#topHosts", '/ajax.txt', { if: "" }, "kb", refresh);
   do_pie("#topApplicationProtocols", '/iface_ndpi_stats.lua', { if: "any" }, "", refresh);
   }

</script>
