--
-- (C) 2014 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

host_ip   = _GET["host"]
host_name = _GET["name"]
host_id   = _GET["id"]

if(mode ~= "embed") then
   sendHTTPHeader('text/html')
   ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")
   active_page = "hosts"
   dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")
end

print("<hr><h2><A HREF=/lua/host_details.lua?host="..host_ip..">"..host_name.."</A> Processes Interaction</H2>")

print [[
<span style="float:right"><a href="javascript:history.go(-1)"><i class='fa fa-reply'></i></a></span>
<div id="chart"></div>

<style>

.link {
  fill: none;
  stroke: #666;
  stroke-width: 1.5px;
}

#licensing {
  fill: green;
}

.link.licensing {
  stroke: green;
}

.link.resolved {
  stroke-dasharray: 0,2 1;
}

circle {
  fill: #ccc;
  stroke: #333;
  stroke-width: 1.5px;
}

text {
  font: 10px sans-serif;
  pointer-events: none;
  text-shadow: 0 1px 0 #fff, 1px 0 0 #fff, 0 -1px 0 #fff, -1px 0 0 #fff;
}

</style>


<script>

var links; // a global
var nodes = {};

]]

print('d3.json("/lua/sprobe_host_process_data.lua?host='..host_ip..'&id='..host_id..'",')


print [[
      function(error, json) {
	    if (error) return console.warn(error);
	    links = json;
	    
	    // Compute the distinct nodes from the links.
	    links.forEach(function(link) {

				if(isNaN(link.source)) {
				   /* IP Address -> PID */
				   _link = "/lua/sprobe_host_process.lua?host="+link.source+"&name="+link.source_name+"&id=0";
				} else {
				   /* PID -> IP Address */
				   _link = "/lua/get_process_info.lua?pid="+link.source+"&name="+link.source_name+"&host=]] print(host_ip) print [[";
				}
				link.source = nodes[link.source] || (nodes[link.source] = {name: link.source_name, link: _link });

				if(isNaN(link.target)) {
				   /* IP Address -> PID */
				   _link = "/lua/sprobe_host_process.lua?host="+link.target+"&name="+link.target_name+"&id=0";
				} else {
				   /* PID -> IP Address */
				   _link = "/lua/get_process_info.lua?pid="+link.target+"&name="+link.target_name+"&host=]] print(host_ip) print [[";
				}

				link.target = nodes[link.target] || (nodes[link.target] = {name: link.target_name, link: _link });
			     });
			  var width = 960,
			  height = 500;
			  
			  var force = d3.layout.force()
			  .nodes(d3.values(nodes))
			  .links(links)
			  .size([width, height])
			  .linkDistance(60)
			  .charge(-300)
			  .on("tick", tick)
			  .start();
			  
			  var svg = d3.select("#chart").append("svg")
			  .attr("width", width)
			  .attr("height", height);

			  // Per-type markers, as they don't inherit styles.
			  svg.append("defs").selectAll("marker")
			  .data(["suit", "licensing", "resolved"])
			  .enter().append("marker")
			  .attr("id", function(d) { return d; })
				.attr("viewBox", "0 -5 10 10")
				.attr("refX", 15)
				.attr("refY", -1.5)
				.attr("markerWidth", 6)
				.attr("markerHeight", 6)
				.attr("orient", "auto")
				.append("path")
				.attr("d", "M0,-5L10,0L0,5");

				var path = svg.append("g").selectAll("path")
				.data(force.links())
				.enter().append("path")
				.attr("class", function(d) { return "link " + d.type; })
				      .attr("marker-end", function(d) { return "url(#" + d.type + ")"; });

					    var circle = svg.append("g").selectAll("circle")
					    .data(force.nodes())
					    .enter().append("circle")
					    .attr("r", 6)
					    .call(force.drag)
					    .on("click", function(d) { 
							       window.location.href = d.link;
							    } );

						var text = svg.append("g").selectAll("text")
						.data(force.nodes())
						.enter().append("text")
						.attr("x", 8)
						.attr("y", ".31em")
						.text(function(d) { return d.name; });

						      // Use elliptical arc path segments to doubly-encode directionality.
						      function tick() {
							    path.attr("d", linkArc);
							    circle.attr("transform", transform);
							    text.attr("transform", transform);
							 }

							 function linkArc(d) {
							       var dx = d.target.x - d.source.x,
							       dy = d.target.y - d.source.y,
							       dr = Math.sqrt(dx * dx + dy * dy);
							       return "M" + d.source.x + "," + d.source.y + "A" + dr + "," + dr + " 0 0,1 " + d.target.x + "," + d.target.y;
							    }

							    function transform(d) {
								  return "translate(" + d.x + "," + d.y + ")";
							       }
							    });
</script>


					    ]]
if(mode ~= "embed") then
dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
end
