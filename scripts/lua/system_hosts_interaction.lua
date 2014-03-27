--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

if(host_ip == nil) then
   host_ip = _GET["host"]
end

if(mode == nil) then
   mode  = _GET["mode"]
end

if(host_name == nil) then
   host_name = _GET["name"]
end

if(mode ~= "embed") then
   sendHTTPHeader('text/html')
   ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")
   active_page = "hosts"
   dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")
end

num_top_hosts = 10

if(host_ip ~= nil) then
   num = 1
else
   interface.find(ifname)
   hosts_stats = interface.getHostsInfo()
   num = 0
   for key, value in pairs(hosts_stats) do
      num = num + 1
   end
end

if(num > 0) then
   if(mode ~= "embed") then
      if(host_ip == nil) then
   print("<hr><h2>Top System Hosts Interaction</H2>")
      else
   name = host_name
   if(name == nil) then name = host_ip end
   print("<hr><h2>"..name.." Interactions</H2><i class=\"fa fa-chevron-left fa-lg\"></i><small><A onClick=\"javascript:history.back()\">Back</A></small>")
      end
   end

print [[
<div id="chart"></div>
<style>

.link {
  fill: none;
  stroke: #666;
  stroke-width: 1.5px;
}

.link-group {
  stroke: rgb(0, 136, 204);
  stroke-width: 0.1px;
}

.node-tooltip {
  font: 10px sans-serif;
  color: #333;
}

#licensing {
  fill: green;
}

.link.licensing {
  stroke: green;
}

.link.resolved {
  stroke-dasharray: 0, 2 1;
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

var width = 960,
    height = 500;

var color = d3.scale.category20();

var force = d3.layout.force()
    .charge(-300)
    .linkDistance(150)
    .size([width, height]);

var svg = d3.select("#chart").append("svg")
    .attr("width", width)
    .attr("height", height);

var explode_process = '';

function refreshGraph() {
d3.json("/lua/get_system_hosts_interaction.lua", function(error, links) {
  var nodes = {};
  var max_node_bytes = 0;
  var max_link_bytes = 0;

  links.forEach(function(link) {
    link.source = (explode_process == link.client_name ? link.client : link.client_name);
    link.target = (explode_process == link.server_name ? link.server : link.server_name);
    if (!nodes[link.source]) nodes[link.source] = {name: link.client_name, id: link.source, bytes: 0};
    if (!nodes[link.target]) nodes[link.target] = {name: link.server_name, id: link.target, bytes: 0};
    nodes[link.source]['bytes'] += link.bytes;
    nodes[link.target]['bytes'] += link.bytes;
    if (nodes[link.source]['bytes'] > max_node_bytes) max_node_bytes = nodes[link.source]['bytes'];
    if (nodes[link.target]['bytes'] > max_node_bytes) max_node_bytes = nodes[link.target]['bytes'];
    if (link.bytes > max_link_bytes) max_link_bytes = link.bytes;
    link.source = nodes[link.source];
    link.target = nodes[link.target];
  });

  force
      .nodes(d3.values(nodes))
      .links(links)
      .on("tick", tick)
      .start();

/*
  svg.append("defs").selectAll("marker")
    .data(force.links())
    .enter().append("marker")
    .attr("id", function(d) { return d.type; })
    .attr("viewBox", "0 -5 10 10")
    .attr("refX", 15)
    .attr("refY", -1.5)
    .attr("markerWidth", 6)
    .attr("markerHeight", 6)
    .attr("orient", "auto")
    .append("path")
    .attr("d", "M0,-5L10,0L0,5");
*/

  var path = svg.selectAll("path")
    .data(force.links())
    .enter().append("g")
    .attr("class", "link-group")
    .append("path")
      .attr("class", function(d) { return "link"; })
      .style("stroke-width", function(d) { return getWeight(d.bytes); })
      /*.attr("marker-end", function(d) { return "url(#" + d.type + ")"; }*/
      .attr("marker-end", "url(#end)")
      .attr("id", function(d, i) { return "link" + i; });
  
  svg.selectAll(".link-group").append("text")
    .attr("dy", "-0.5em")
    .append("textPath")
    .attr("startOffset",function(d,i) { return "20%"; })
    .attr("xlink:href", function(d,i) { return "#link" + i; })
    .text(function(d) { return bytesToVolume(d.cli2srv_bytes) + " | " + bytesToVolume(d.srv2cli_bytes); });

  var tooltip = d3.select("#chart")
    .append("div")
    .attr("class", "node-tooltip")
    .style("position", "absolute")
    .style("z-index", "10")
    .style("visibility", "hidden")
    .text("");

  var node = svg.selectAll(".node")
    .data(force.nodes())
    .enter().append("circle")
    .attr("class", "circle")
    .attr("r", function(d) { return getRadius(d.bytes); })
    .style("fill", function(d) { return color(d.name); })
    .call(force.drag)
    .on("dblclick", function(d) { 
      svg.selectAll("circle").remove(); 
      svg.selectAll("path").remove(); 
      svg.selectAll("text").remove(); 
      if (explode_process == d.name) 
        explode_process = '';
      else
        explode_process = d.name;
      refreshGraph();
    } )
    .on("mouseover", function(d){ tooltip.text(d.id); return tooltip.style("visibility", "visible"); })
    .on("mousemove", function(){ return tooltip.style("top", (event.pageY - 10) + "px").style("left", (event.pageX + 10) + "px"); })
    .on("mouseout",  function(){ return tooltip.style("visibility", "hidden");});

  var text = svg.append("g").selectAll("text")
    .data(force.nodes())
    .enter().append("text")
    .attr("x", 8)
    .attr("y", ".31em")
    .text(function(d) { return d.name; });

  function tick() {
    path.attr("d", linkArc);
    node.attr("transform", transform);
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

  function getWeight(size) {
    var weight = (size ? Math.sqrt((size / max_link_bytes) * 100) / Math.PI : 1);
    if (weight < 1) weight = 1;
    return weight;
  }

  function getRadius(size) {
    var radius = 10 * (size ? Math.sqrt((size / max_node_bytes) * 100) / Math.PI : 1);
    if (radius < 5) radius = 5;
    return radius;
  }

});
}

refreshGraph();

</script>

<p>&nbsp;<p><small><b>NOTE</b></small>
<ol>
]]

-- if(host_ip ~= nil) then
   -- print('<li><small>This map is centered on host <font color="#fd8d3c">'.. host_ip)
   -- if(host_name ~= nil) then print('('.. host_name .. ')') end
   -- print('</font>. Clicking on this host you will visualize its details.</small></li>\n')
-- else
   print('<li><small>This map depicts the interactions of the top  '.. num_top_hosts .. ' hosts.</small></li>\n')
--end
   print('<li><small>Color node: <font color=#aec7e8><b>System Host</b></font>, <font color=#bcbd22><b>Host</b></font>.</small></li>\n')
   print('<li><small>Color link: <font color=orange><b>Incoming</b></font>, <font color=green><b>Outgoing</b></font>,<font color=gray><b>Loopback</b></font>.</small></li>\n')
print [[
<!-- <li> <small>Click is enabled only for hosts that have not been purged from memory.</small></li> -->
</ol>


]]
else
print("<div class=\"alert alert-error\"><img src=/img/warning.png> No results found</div>")
end

if(mode ~= "embed") then
dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
end
