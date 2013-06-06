--
-- (C) 2013 - ntop.org
--



function breakdownBar(sent, sentLabel, rcvd, rcvdLabel)
   if((sent+rcvd) > 0) then
      sent2rcvd = round((sent * 100) / (sent+rcvd), 0)
      print('<div class="progress"><div class="bar bar-warning" style="width: ' .. sent2rcvd.. '%;">'..sentLabel)
      print('</div><div class="bar bar-info" style="width: ' .. (100-sent2rcvd) .. '%;">' .. rcvdLabel .. '</div></div>')
   else
      print('&nbsp;')
   end
end


function drawRRD(host, rrdFile, zoomLevel, baseurl, show_timeseries)
   rrdname = ntop.getDataDir() .. "/rrd/" .. host .. "/" .. rrdFile
   names =  {}
   series = {}
   vals = {
      { "5m", "now-300s" },
      { "10m", "now-600s" },
      { "1h", "now-1h" },
      { "3h", "now-3h" },
      { "6h", "now-6h" },
      { "12h", "now-12h" },
      { "1d", "now-1d" },
      { "1w", "now-1w" },
      { "2w", "now-2w" },
      { "1m", "now-1mon" },
      { "6m", "now-6mon" },
      { "1y", "now-1y" }
   }

   if(zoomLevel == nil) then
      zoomLevel = "1h"
   end

   for k,v in ipairs(vals) do
      if(vals[k][1] == zoomLevel) then
	 start_time = vals[k][2]
      end
   end

   --start_time = "now-10s"
   end_time = "now"

   local maxval_bits_time = 0
   local maxval_bits = 0
   local minval_bits = 0
   local minval_bits_time = 0
   local lastval_bits = 0
   local lastval_bits_time = 0
   local total_bytes = 0
   local num_points = 0
   local step = 1

   prefixLabel = l4Label(string.gsub(rrdFile, ".rrd", ""))

   if(prefixLabel == "bytes") then
      prefixLabel = "Traffic"
   end

   if(ntop.exists(rrdname)) then
      local fstart, fstep, fnames, fdata = rrd.fetch(rrdname, '--start', start_time, '--end', end_time, 'AVERAGE')
      local max_num_points = 600 -- This is to avoid having too many points and thus a fat graph
      local num_points_found = table.getn(fdata)
      local sample_rate = round(num_points_found / max_num_points)

      if(sample_rate < 1) then
	 sample_rate = 1
      end

      step = fstep
      num = 0
      for i, n in ipairs(fnames) do
	 names[num] = prefixLabel
	 if(prefixLabel ~= firstToUpper(n)) then names[num] = names[num] .. " " .. firstToUpper(n) end
	 num = num + 1
      end

      id = 0
      fend = 0
      sampling = 0
      sample_rate = sample_rate-1
      for i, v in ipairs(fdata) do
	 s = {}
	 s[0] = fstart + (i-1)*fstep
	 num_points = num_points + 1

	 local elemId = 1
	 for _, w in ipairs(v) do
	    if(w ~= w) then
	       -- This is a NaN
	       v = 0
	    else
	       v = tonumber(w)
	       if(v < 0) then
		  v = 0
	       end
	    end

	    if(v > 0) then
	       lastval_bits_time = s[0]
	       lastval_bits = v
	    end

	    s[elemId] = v*8 -- bps
	    elemId = elemId + 1
	 end

	 total_bytes = total_bytes + v*fstep
	 -- print(" | " .. (v*fstep) .." |\n")

	 if(sampling == sample_rate) then
	    series[id] = s
	    id = id + 1
	    sampling = 0
	 else
	    sampling = sampling + 1
	 end
      end


   for key, value in pairs(series) do
      local t = 0

      for elemId=0,(num-1) do
	 -- print(">"..value[elemId+1].. "<")
	 t = t + value[elemId+1] -- bps
      end

      t = t * step

      if((minval_bits_time == 0) or (minval_bits >= t)) then
	 minval_bits_time = value[0]
	 minval_bits = t
      end

      if((maxval_bits_time == 0) or (maxval_bits <= t)) then
	 maxval_bits_time = value[0]
	 maxval_bits = t
      end
   end

print [[

<style>
#chart_container {
display: inline-block;
font-family: Arial, Helvetica, sans-serif;
}
#chart {
   float: left;
}
#legend {
   float: left;
   margin-left: 15px;
   color: black;
   background: white;
}
#y_axis {
   float: left;
   width: 40px;
}

</style>

<div class="row">
 <div class="span1"></div>
]]


if(show_timeseries == 1) then
   print [[

<div class="btn-group">
  <button class="btn btn-small dropdown-toggle" data-toggle="dropdown">Timeseries <span class="caret"></span></button>
  <ul class="dropdown-menu">
]]

print('<li><a  href="'..baseurl .. '&rrd_file=' .. "bytes.rrd" .. '&graph_zoom=' .. zoomLevel ..'">'.. "Traffic" ..'</a></li>\n')
print('<li class="divider"></li>\n')

rrds = ntop.readdir(ntop.getDataDir() .. "/rrd/" .. host)

for k,v in pairsByKeys(rrds, asc) do
   proto = string.gsub(rrds[k], ".rrd", "")
      
   if(proto ~= "bytes") then
      label = l4Label(proto)
      print('<li><a href="'..baseurl .. '&rrd_file=' .. rrds[k] .. '&graph_zoom=' .. zoomLevel ..'">'.. label ..'</a></li>\n')
   end
end

print [[
  </ul>
</div><!-- /btn-group -->
]]

end

print('&nbsp; <div class="btn-group" data-toggle="buttons-radio" id="graph_zoom">\n')

for k,v in ipairs(vals) do
   print('<a class="btn btn-small ')

   if(vals[k][1] == zoomLevel) then
      print("active")
   end

   print('" href="'..baseurl .. '&rrd_file=' .. rrdFile .. '&graph_zoom=' .. vals[k][1] ..'">'.. vals[k][1] ..'</a>\n')
end

print [[
</div>
</div>

<div class="row">
&nbsp;
</div>

<div class="row">
 <div class="span1"></div>
<div id="chart_container">
   <div id="y_axis"></div>
   <div id="chart"></div>

   <table border=0>
   <tr><td><div id="legend"></div></td><td><div id="chart_legend"></div></td></tr>
   <tr><td colspan=2>

   <table class="table table-bordered table-striped">
   ]]


print('   <tr><th>' .. prefixLabel .. '</th><th>Time</th><th>Value</th></tr>\n')
print('   <tr><th>Min</th><td>' .. os.date("%x %X", minval_bits_time) .. '</td><td>' .. bitsToSize(minval_bits/step) .. '</td></tr>\n')
print('   <tr><th>Max</th><td>' .. os.date("%x %X", maxval_bits_time) .. '</td><td>' .. bitsToSize(maxval_bits/step) .. '</td></tr>\n')
print('   <tr><th>Last</th><td>' .. os.date("%x %X", last_time) .. '</td><td>' .. bitsToSize(lastval_bits/step)  .. '</td></tr>\n')
print('   <tr><th>Average</th><td colspan=2>' .. bitsToSize(total_bytes*8/(step*num_points)) .. '</td></tr>\n')
print('   <tr><th>Total Traffic</th><td colspan=2>' .. bytesToSize(total_bytes)..  '</td></tr>\n')


print [[
   </table>

   </td></tr>
   </table>
</div>

</div>

<script>


var palette = new Rickshaw.Color.Palette();

var graph = new Rickshaw.Graph( {
				   element: document.getElementById("chart"),
				   width: 600,
				   height: 300,
				   renderer: 'area',
				   series: [

				]]

				if(names ~= nil) then
for elemId=0,(num-1) do
   if(elemId > 0) then
      print ","
   end

   print ("{\nname: '".. names[elemId] .. "',\n")

   print("color: palette.color(),\ndata: [\n")

   n = 0
   for key, value in pairs(series) do
      if(n > 0) then
	 print(",\n")
      end
      print ("\t{ x: "..  value[0] .. ", y: ".. value[elemId+1] .. " }")
      n = n + 1
   end

      print("\n]}\n")
end
end

print [[
				   ]
				} );

graph.render();

var chart_legend = document.querySelector('#chart_legend');


function fdate(when) {
      var epoch = when*1000;
      var d = new Date(epoch);

      return(d);
}

function fbits(bits) {
		      var sizes = ['bps', 'Kbit', 'Mbit', 'Gbit', 'Tbit'];
		      if (bits == 0) return 'n/a';
		      var i = parseInt(Math.floor(Math.log(bits) / Math.log(1024)));
		      return Math.round(bits / Math.pow(1024, i), 2) + ' ' + sizes[i];
}

var Hover = Rickshaw.Class.create(Rickshaw.Graph.HoverDetail, {
    graph: graph,
    xFormatter: function(x) { return new Date( x * 1000 ); },
    yFormatter: function(bits) { return(fbits(bits)); },
    render: function(args) {
		var graph = this.graph;
		var points = args.points;
		var point = points.filter( function(p) { return p.active } ).shift();

		if (point.value.y === null) return;

		var formattedXValue = fdate(point.value.x); // point.formattedXValue;
		var formattedYValue = fbits(point.value.y); // point.formattedYValue;

		this.element.innerHTML = '';
		this.element.style.left = graph.x(point.value.x) + 'px';

		var xLabel = document.createElement('div');

		xLabel.className = 'x_label';
		xLabel.innerHTML = formattedXValue;
		this.element.appendChild(xLabel);

		var item = document.createElement('div');

		item.className = 'item';
		item.innerHTML = this.formatter(point.series, point.value.x, point.value.y, formattedXValue, formattedYValue, point);
		item.style.top = this.graph.y(point.value.y0 + point.value.y) + 'px';

		this.element.appendChild(item);

		var dot = document.createElement('div');

		dot.className = 'dot';
		dot.style.top = item.style.top;
		dot.style.borderColor = point.series.color;

		this.element.appendChild(dot);

		if (point.active) {
			item.className = 'item active';
			dot.className = 'dot active';
		}

		this.show();

		if (typeof this.onRender == 'function') {
			this.onRender(args);
		}

		// Put the selected graph epoch into the legend
		//chart_legend.innerHTML = point.value.x; // Epoch
		event
	}
} );

var hover = new Hover( { graph: graph } );

var legend = new Rickshaw.Graph.Legend( {
					   graph: graph,
					   element: document.getElementById('legend')
					} );

//var axes = new Rickshaw.Graph.Axis.Time( { graph: graph } ); axes.render();

var yAxis = new Rickshaw.Graph.Axis.Y({
    graph: graph,
    tickFormat: Rickshaw.Fixtures.Number.formatKMBT
				      });

yAxis.render();


</script>

]]
 else
   print("<div class=\"alert alert-error\"><img src=/img/warning.png> This archive file cannot be found</div>")

end
end