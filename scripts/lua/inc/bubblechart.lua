--
-- (C) 2013 - ntop.org
--

print [[

<style>


</style>
<div id="bubble_chart"></div>
<script charset="utf-8" src="/js/d3.v3.js"></script>
<script>
function bubble() {
var margin = {top: 1, right: 1, bottom: 6, left: 1},
    width = 600 - margin.left - margin.right,
    height = 400 - margin.top - margin.bottom;

var diameter = 240,
    format = d3.format(",d"),
    color = d3.scale.category20c();


var bubble = d3.layout.pack()
    .sort(null)
    .size([diameter, diameter])
    .padding(1.5);

d3.select("#bubble_chart").select("svg").remove();

var svg_bubble = d3.select("#bubble_chart").append("svg")
    .attr("width", width)
    .attr("height", height)
    .attr("class", "bubble");
]]

if((_GET["hosts"] ~= nil) and (_GET["aggregation"] ~= nil))then
  print('d3.json("/lua/hosts_comparison_bubble.lua?hosts='.._GET["hosts"] .. '&aggregation='.._GET["aggregation"] ..' "')
  
elseif(_GET["hosts"] ~= nil) then
  print('d3.json("/lua/hosts_comparison_bubble.lua?hosts='.._GET["hosts"] ..' "')
else
  print('d3.json("/lua/hosts_comparison_bubble.lua"')
end

-- print('d3.json("/flare.json" ')
print [[
, function(error, root) {

  var node = svg_bubble.selectAll(".node")
      .data(bubble.nodes(classes(root))
      .filter(function(d) { return !d.children; }))
      .enter().append("g")
      .attr("class", "node")
      .attr("transform", function(d) { return "translate(" + d.x + "," + d.y + ")"; });


  node.append("title")
      .text(function(d) { return d.className + ": " + bytesToVolume(d.value) + "\n Double click to show more information about this flows."; });

  node.append("circle")
      .attr("r", function(d) { return (d.r); })
      .style("fill", function(d) { return color(d.r); })
      .on("dblclick", function(d) { window.location.href = "/lua/flows_stats.lua?hosts=]]

print(_GET["hosts"])

print [[&aggregation="+escape(d.aggregation)+"&key="+escape(d.key) ; });

  node.append("text")
      .attr("dy", ".3em")
      .style("text-anchor", "middle")
      .text(function(d) { return d.className.substring(0, d.r / 3); });
});



// Returns a flattened hierarchy containing all leaf nodes under the root.
function classes(root) {
  var classes = [];

  function recurse(name, node) {
    if (node.children) node.children.forEach(function(child) { recurse(node.name, child); });
    else classes.push({packageName: name, className: node.name, value: node.size, aggregation: node.aggregation, key: node.key});
  }

  recurse(null, root);
  return {children: classes};
}

d3.select(self.frameElement).style("height", diameter + "px");
}

bubble();

// Refresh every 5 seconds
var bubble_interval = window.setInterval(bubble, 5000);
</script>

]]    
