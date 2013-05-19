--
-- (C) 2013 - ntop.org
--

function drawRRD(host, rrdFile, zoomLevel, baseurl)

   rrdname = ntop.getDataDir() .. "/rrd/" .. host .. "/" .. rrdFile
   names =  {}
   series = {}
   vals = {  
      { "10m", "now-10m" }, 
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

   end_time = "now"

   local maxval_time = 0
   local maxval = 0
   local minval = 0
   local minval_time = 0
   local lastval = 0
   local lastval_time = 0
   
   prefixLabel = string.gsub(rrdFile, ".rrd", "")
   
   if(prefixLabel == "bytes") then
      prefixLabel = "Traffic"
   end

   if(ntop.exists(rrdname)) then
      local fstart, fstep, fnames, fdata = rrd.fetch(rrdname, '--start', start_time, '--end', end_time, 'AVERAGE')
     
      num = 0
      for i, n in ipairs(fnames) do
	 names[num] = prefixLabel .. " " .. firstToUpper(n)
	 num = num + 1
      end
      
      id = 0
      for i, v in ipairs(fdata) do
	 s = {}
	 s[0] = fstart + (i-1)*fstep
	 
	 local elemId = 1
	 for _, w in ipairs(v) do	 
	    if(w ~= w) then
	       -- This is a NaN
	       v = 0
	    else
	       v = tonumber(w)*8
	       if(v < 0) then
		  v = 0
	       end
	    end
 	 
	    lastval_time = s[0] 
	    lastval = v
	    
	    s[elemId] = v
	    elemId = elemId + 1
      end
      
      series[id] = s
      id = id + 1
   end
end

for key, value in pairs(series) do
   local t = 0

   for elemId=0,(num-1) do
      t = t + value[elemId+1]
   end
   
   if((minval_time == 0) or (minval >= t)) then
      minval_time = value[0] 
      minval = t
   end
   
   if((maxval_time == 0) or (maxval <= t)) then
      maxval_time = value[0] 
      maxval = t
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
      print('<li><a href="'..baseurl .. '&rrd_file=' .. rrds[k] .. '&graph_zoom=' .. zoomLevel ..'">'.. string.gsub(rrds[k], ".rrd", "") ..'</a></li>\n')
   end
end

print [[
  </ul>
</div><!-- /btn-group -->


&nbsp; <div class="btn-group" data-toggle="buttons-radio" id="graph_zoom">

]]


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
   <tr><td><div id="legend"></div></td></tr>
   <tr><td>

   <table class="table table-bordered table-striped">
   ]]


print('   <tr><th>' .. prefixLabel .. '</th><th>Time</th><th>Value</th></tr>\n')
print('   <tr><th>Min Value</th><td>' .. os.date("%x %X", minval_time) .. '</td><td>' .. bitsToSize(minval) .. '</td></tr>\n')
print('   <tr><th>Max Value</th><td>' .. os.date("%x %X", maxval_time) .. '</td><td>' .. bitsToSize(maxval) .. '</td></tr>\n')
print('   <tr><th>Last Value</th><td>' .. os.date("%x %X", last_time) .. '</td><td>' .. bitsToSize(lastval)  .. '</td></tr>\n')

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

var hoverDetail = new Rickshaw.Graph.HoverDetail( {
						     graph: graph,
    xFormatter: function(x) { return new Date( x * 1000 ); },
    yFormatter: function(bits) { 	
		      var sizes = ['bps', 'Kbit', 'Mbit', 'Gbit', 'Tbit'];
		      if (bits == 0) return 'n/a';
		      var i = parseInt(Math.floor(Math.log(bits) / Math.log(1024)));
		      return Math.round(bits / Math.pow(1024, i), 2) + ' ' + sizes[i]; }
						  } );

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

end
