--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

sendHTTPHeader('text/html')


ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")
active_page = "hosts"
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

interface.find(ifname)
hosts_stats = interface.getHostsInfo()
num = 0
for key, value in pairs(hosts_stats) do
    num = num + 1
end


if(num > 0) then
print [[

<hr>
<h2>Hosts Interaction</H2>



<style>


svg {
 font: 10px sans-serif;
}

.axis path, .axis line {
 fill: none;
 stroke: #000;
 shape-rendering: crispEdges;
}

sup, sub {
  line-height: 0;
}

    q:before, blockquote:before {
 content: "?";
}

    q:after, blockquote:after {
 content: "?";
}

    blockquote:before {
 position: absolute;
 left: 2em;
}

    blockquote:after {
 position: absolute;
}

  </style>
  <style>
#chart {
height: 600px;
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

       circle.node-dot {
       fill: DarkSlateGray;
       stroke: SlateGray;
       stroke-width: 1px;
       }


       path.link {
       fill: none;
       stroke: SlateGray;
       stroke-width: 1.5px;
       }

       marker#defaultMarker {
       fill: SlateGray;
       }

       path.link.defaultMarker {
       stroke: SlateGray;
       }

       circle {
       fill: #ccc;
       stroke: #333;
       stroke-width: 1.5px;
       }

       text {
	 pointer-events: none;
       }

       text.shadow {
       stroke: #fff;
       stroke-width: 3px;
       stroke-opacity: .8;
       }

       </style><style>path.link.proposer{stroke:red;}
       marker#bus{fill:blue;}
       marker#manual{fill:red;}
       path.link.direct{stroke:green;  }
       path.link.bus{stroke:blue;} 
       path.link.manual{stroke:red;stroke-dasharray: 0, 2 1;} </style><script>
       /**
	* do the force vizualization
	* @param {string} divName name of the div to hold the tree
	* @param {object} inData the source data
	*/
       function doTheTreeViz(divName, inData) {
	 // tweak the options
	 var options = $.extend({
	   stackHeight : 12,
	       radius : 5,
	       fontSize : 12,
	       labelFontSize : 8,
	       nodeLabel : null,
	       markerWidth : 0,
	       markerHeight : 0,
	       width : $(divName).outerWidth(),
	       gap : 1.5,
	       nodeResize : "",
	       linkDistance : 30,
	       charge : -120,
	       styleColumn : null,
	       styles : null,
	       linkName : null,
	       height : $(divName).outerHeight()
	       }, inData.d3.options);
	 // set up the parameters
	 options.gap = options.gap * options.radius;
	 var width = options.width;
	 var height = options.height;
	 var data = inData.d3.data;
	 var nodes = data.nodes;
	 var links = data.links;
	 var color = d3.scale.category20();

	 var force = d3.layout.force().nodes(nodes).links(links).size([width, height]).linkDistance(options.linkDistance).charge(options.charge).on("tick", tick).start();

	 var svg = d3.select(divName).append("svg:svg").attr("width", width).attr("height", height);

	 // get list of unique values in stylecolumn
	 linkStyles = [];
	 if (options.styleColumn) {
	   var x;
	   for (var i = 0; i < links.length; i++) {
	     if (linkStyles.indexOf( x = links[i][options.styleColumn].toLowerCase()) == -1)
	       linkStyles.push(x);
	   }
	 } else
	   linkStyles[0] = "defaultMarker";

	 // do we need a marker?

	 if (options.markerWidth) {
	   svg.append("svg:defs").selectAll("marker").data(linkStyles).enter().append("svg:marker").attr("id", String).attr("viewBox", "0 -5 10 10").attr("refX", 15).attr("refY", -1.5).attr("markerWidth", options.markerWidth).attr("markerHeight", options.markerHeight).attr("orient", "auto").append("svg:path").attr("d", "M0,-5L10,0L0,5");
	 }

	 var path = svg.append("svg:g").selectAll("path").data(force.links()).enter().append("svg:path").attr("class", function(d) {
	     return "link " + (options.styleColumn ? d[options.styleColumn].toLowerCase() : linkStyles[0]);
	   }).attr("marker-end", function(d) {
	       return "url(#" + (options.styleColumn ? d[options.styleColumn].toLowerCase() : linkStyles[0] ) + ")";
	     });

	 var circle = svg.append("svg:g").selectAll("circle").data(force.nodes()).enter().append("svg:circle").attr("r", function(d) {
	     return getRadius(d);
	   }).style("fill", function(d) {
	       return color(d.group);
	     }).call(force.drag);

	 if (options.nodeLabel) {
	   circle.append("title").text(function(d) {
	       return d[options.nodeLabel];
	     });
	 }
    
	 if (options.linkName) {
	   path.append("title").text(function(d) {
	       return d[options.linkName];
	     });
	 }
	 var text = svg.append("svg:g").selectAll("g").data(force.nodes()).enter().append("svg:g");

	 // A copy of the text with a thick white stroke for legibility.
	 text.append("svg:text").attr("x", options.labelFontSize).attr("y", ".31em").attr("class", "shadow").text(function(d) {
	     return d[options.nodeLabel];
	   });

	 text.append("svg:text").attr("x", options.labelFontSize).attr("y", ".31em").text(function(d) {
	     return d[options.nodeLabel];
	   });
	 function getRadius(d) {
	   return options.radius * (options.nodeResize ? Math.sqrt(d[options.nodeResize]) / Math.PI : 1);
	 }

	 // Use elliptical arc path segments to doubly-encode directionality.
	 function tick() {
	   path.attr("d", function(d) {
	       var dx = d.target.x - d.source.x, dy = d.target.y - d.source.y, dr = Math.sqrt(dx * dx + dy * dy);
	       return "M" + d.source.x + "," + d.source.y + "A" + dr + "," + dr + " 0 0,1 " + d.target.x + "," + d.target.y;
	     });

	   circle.attr("transform", function(d) {
	       return "translate(" + d.x + "," + d.y + ")";
	     });

	   text.attr("transform", function(d) {
	       return "translate(" + d.x + "," + d.y + ")";
	     });
	 }

       }
       </script><script> 

window['ntopData'] = {"d3":{"options":
			    {"radius":"24","fontSize":"30","labelFontSize":"30","charge":"-600","nodeResize":"count","nodeLabel":"label","markerHeight":"6","markerWidth":"6","styleColumn":"styleColumn","linkName":"group"},
		       "data":{"links":[

]]

-- Nodes

interface.find(ifname)
hosts_stats = getTopInterfaceHosts(10, true)

hosts_id = {}
ids = {}

num = 0
links = 0
for key, values in pairs(hosts_stats) do
   if(values["localhost"] ~= nil) then
      host = interface.getHostInfo(key)

      if(host ~= nil) then
      if(hosts_id[key] == nil) then
	 hosts_id[key] = { }
	 hosts_id[key]['count'] = 0
	 hosts_id[key]['id'] = num
	 hosts_id[key]['localhost'] = 1
	 ids[num] = key
	 key_id = num
	 num = num + 1
	 hosts_id[key]["name"] = host["name"]
	 if(hosts_id[key]["name"] == nil) then hosts_id[key]["name"] = ntop.getResolvedAddress(key) end
      else
	 key_id = hosts_id[key]['id']
      end


      if(host["contacts"]["client"] ~= nil) then
	 for k,v in pairs(host["contacts"]["client"]) do 
	    if(hosts_id[k] == nil) then
	       hosts_id[k] = { }
	       hosts_id[k]['count'] = 0
	       hosts_id[k]['id'] = num
	       ids[num] = k
	       peer_id = num
	       num = num + 1
	    else
	       peer_id = hosts_id[k]['id']
	    end
	    hosts_id[key]['count'] = hosts_id[key]['count'] + v
	    if(links > 0) then print(",") end
	    print('\n{"source":'..key_id..',"target":'..peer_id..',"depth":6,"count":'..v..',"styleColumn":"client","linkName":""}')
	    links = links + 1
	 end
      end

      if(host["contacts"]["server"] ~= nil) then
	 for k,v in pairs(host["contacts"]["server"]) do 
	    if(hosts_id[k] == nil) then
	       hosts_id[k] = { }
	       hosts_id[k]['count'] = 0
	       hosts_id[k]['id'] = num
	       ids[num] = k
	       peer_id = num
	       num = num + 1
	    else
	       peer_id = hosts_id[k]['id']
	    end
	    hosts_id[key]['count'] = hosts_id[key]['count'] + v
	    if(links > 0) then print(",") end
	    print('\n{"source":'..key_id..',"target":'..peer_id..',"depth":6,"count":'..v..',"styleColumn":"server","linkName":""}')
	    links = links + 1
	 end
	 end
      end
   end
end

tot_hosts = num

print [[

],"nodes":[
]]

-- Nodes

maxval = 0
for k,v in pairs(hosts_id) do 
   if(v['count'] > maxval) then maxval = v['count'] end
end

num = 0
for i=0,tot_hosts-1 do
   k = ids[i]
   v = hosts_id[k]
   key = k
   name = v["name"]
   if(name == nil) then name = k end
   tot = math.floor(0.5+(v['count']*100)/maxval)
   if(tot < 1) then tot = 1 end
   if(v['localhost'] ~= nil) then label = "local" else label = "remote" end
   if(num > 0) then print(",") end
   print('\n{"name":"'.. key ..'","count":'.. tot ..',"group":"' .. label .. '","linkCount": '.. tot .. ',"label":"' .. name ..'", "url" : "/lua/host_details.lua?host='.. key.. '"}')
   num = num + 1
end



print [[

	   ]
}}};


</script></head><body>
  <div id="chart">

  </div>


<!-- http://ramblings.mcpher.com/Home/excelquirks/d3 -->
  <script type="text/javascript">
       doTheTreeViz("#chart", ntopData);
  </script>





]]
else
print("<div class=\"alert alert-error\"><img src=/img/warning.png> No results found</div>")
end

dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
