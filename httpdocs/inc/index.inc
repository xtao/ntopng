
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
        <h4>Top Talkers Relationships</h4>
<style>


#chart {
  height: 380px;
}

.node rect {
  cursor: move;
  fill-opacity: .9;
  shape-rendering: crispEdges;
}

.node text {
  pointer-events: none;
  text-shadow: 0 1px 0 #fff;
}

.link {
  fill: none;
  stroke: #000;
  stroke-opacity: .2;
}

.link:hover {
  stroke-opacity: .5;
}

</style>

<p id="chart">
<script src="/js/sankey.js"></script>

<script>

var margin = {top: 1, right: 1, bottom: 6, left: 1},
    width = 800 - margin.left - margin.right,
    height = 400 - margin.top - margin.bottom;

function sankey() {
  var formatNumber = d3.format(",.0f"),
    format = function(sent, rcvd) { return "[sent: "+formatNumber(sent)+", rcvd: "+formatNumber(rcvd)+"]"; },
    color = d3.scale.category20();
    
    d3.json("/iface_flows_sankey.lua", function(hosts) {
	d3.select("#chart").select("svg").remove();

	//	var svg = d3.select("#chart").append("svg")

	var svg = d3.select("#chart").append("svg")
	  .attr("width", width + margin.left + margin.right)
	  .attr("height", height + margin.top + margin.bottom)
	  .append("g")
	  .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

	var sankey = d3.sankey()
	  .nodeWidth(15)
	  .nodePadding(10)
	  .size([width, height]);

	var path = sankey.link();
	sankey
	  .nodes(hosts.nodes)
	  .links(hosts.links)
	  .layout(32);

	/* Color the link according to traffic prevalence */
	var colorlink = function(d){
	  if (d.sent > d.rcvd) return color(d.source.name);
	  else return color(d.target.name);
	}

	var link = svg.append("g").selectAll(".link")
	  .data(hosts.links)
	  .enter().append("path")
	  .attr("class", "link")
	  .attr("d", path)
	  .style("stroke-width", function(d) { return Math.max(1, d.dy); })
	  .style("stroke", function(d){ return d.color = colorlink(d); })
	  .sort(function(a, b) { return b.dy - a.dy; });

	link.append("title")
	  .text(function(d) { return d.source.name + " - " + d.target.name + "\n" + format(d.sent, d.rcvd); });

	var node = svg.append("g").selectAll(".node")
	  .data(hosts.nodes)
	  .enter().append("g")
	  .attr("class", "node")
	  .attr("transform", function(d) { return "translate(" + d.x + "," + d.y + ")"; })
	  .call(d3.behavior.drag()
		.origin(function(d) { return d; })
		.on("dragstart", function() { this.parentNode.appendChild(this); })
		.on("drag", dragmove));


	node.append("rect")
	  .attr("height", function(d) { return d.dy; })
	  .attr("width", sankey.nodeWidth())
	  .style("fill", function(d) { return d.color = color(d.name.replace(/ .*/, "")); })
	  .style("stroke", function(d) { return d3.rgb(d.color).darker(2); })
	  .append("title")
	  .text(function(d) { return d.name + "\n" + format(d.value); });

	node.append("text")
	  .attr("x", -6)
	  .attr("y", function(d) { return d.dy / 2; })
	  .attr("dy", ".35em")
	  .attr("text-anchor", "end")
	  .attr("transform", null)
	  .text(function(d) { return d.name; })
	  .filter(function(d) { return d.x < width / 2; })
	  .attr("x", 6 + sankey.nodeWidth())
	  .attr("text-anchor", "start");

	function dragmove(d) {
	  d3.select(this).attr("transform", "translate(" + d.x + "," + (d.y = Math.max(0, Math.min(height - d.dy, d3.event.y))) + ")");
	  sankey.relayout();
	  link.attr("d", path);
	}
      });
}

sankey();

// Refresh every 5 seconds
window.setInterval(sankey, 5000);


</script>
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
