--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "graph_utils"

sendHTTPHeader('text/html')

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")

mode = _GET["mode"]
if(mode == nil) then mode = "all" end

active_page = "hosts"
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

print [[
  <br>
  <br>
  <!-- Left Tab -->
  <div class="tabbable tabs-left">

    <ul class="nav nav-tabs">
      <li class="active"><a href="#Overview" data-toggle="tab">Overview</a></li>
      <li ><a href="#Timeline" data-toggle="tab">Timeline</a></li>
    </ul>

    <!-- Tab content-->
    <div class="tab-content">
]]

print [[
      <div class="tab-pane active" id="Overview">

      <div id="table-hosts"></div>
   <script>
   $("#table-hosts").datatable({
          title: "Active Processes",
          url: "/lua/get_processes_data.lua",
          ]]

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/processes_stats.inc")
print 
[[     </div> <!-- Tab Overview-->
]]


print [[
  
  <link type="text/css" rel="stylesheet" href="http://ajax.googleapis.com/ajax/libs/jqueryui/1.8/themes/base/jquery-ui.css">
  <link type="text/css" rel="stylesheet" href="/css/Rickshaw/graph.css">
  <link type="text/css" rel="stylesheet" href="/css/Rickshaw/detail.css">
  <link type="text/css" rel="stylesheet" href="/css/Rickshaw/legend.css">

  <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.6.2/jquery.min.js"></script>
  <script>
    jQuery.noConflict();
  </script>

  <script src="https://ajax.googleapis.com/ajax/libs/jqueryui/1.8.15/jquery-ui.min.js"></script>

  <script src="/js/Rickshaw/Rickshaw.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Class.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Compat.ClassList.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Graph.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Graph.Renderer.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Graph.Renderer.Area.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Graph.Renderer.Line.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Graph.Renderer.Bar.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Graph.Renderer.ScatterPlot.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Graph.Renderer.Stack.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Graph.RangeSlider.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Graph.RangeSlider.Preview.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Graph.HoverDetail.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Graph.Annotate.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Graph.Legend.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Graph.Axis.Time.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Graph.Behavior.Series.Toggle.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Graph.Behavior.Series.Order.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Graph.Behavior.Series.Highlight.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Graph.Smoother.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Fixtures.Time.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Fixtures.Time.Local.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Fixtures.Number.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Fixtures.Color.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Color.Palette.js"></script>
  <script src="/js/Rickshaw/Rickshaw.Graph.Axis.Y.js"></script>

<style>
div, span, p, td {
  font-family: Arial, sans-serif;
}
#content {
  width: 1200px;
}
#chart {
  display: inline-block;
}
#chart path {
  -webkit-transition: opacity 0.2s linear;
}
#preview {
  margin-top: 10px;
}
#legend {
  display: inline-block;
  position: relative;
  left: 8px;
}
#legend_container {
  position: absolute;
  right: 0;
  bottom: 26px;
  width: 0;
}
#chart_container {
  position: relative;
  display: inline-block;
}
.rickshaw_graph .detail {
  left: -1000;
}
#chart {
  border: 1px solid #f0f0f0;
}
#side_panel {
  padding: 0 20px 20px 0;
  width: 240px;
  display: inline-block;
  vertical-align: top;
}
#side_panel section {
  color: #505050;
  font-size: 12px;
}
#side_panel section h6 {
  margin: 0 0 1em 0;
  font-size: 12px;
  font-weight: normal;
}
#side_panel .ui-slider-horizontal {
  height: 1px !important;
  border-color: #e0e0e0;
  margin-bottom: 10px;
}
#side_panel .ui-slider-handle {
  border-color: #a0a0a0;
  height: 9px !important;
  width: 9px !important;
  top: -5px !important;
  border-radius: 6px;
  outline: none;
  cursor: pointer;
}
#legend {
  background-color: white;
  margin-left: 0;
  padding: 0;
  left: 0;
}
#legend .label {
  color: #404040;
}
#legend .action {
  color: black;
  opacity: 0.5;
}
#legend ul {
  padding: 0;
}
h1 {
  font-family: Franklin Gothic Medium, UnDotum, Helvetica, Arial;
  font-weight: normal;
  font-size: 20px;
}
section {
  border: none;
  border-top: 1px solid #eaeaea;
  padding: 15px 0;
}

#renderer_form.toggler {
  display: block;
  margin: 0;
}
#renderer_form.toggler input[type=radio]:checked {
  outline: 2px solid steelblue;
}
#renderer_form.toggler input[type=radio] {
  -moz-appearance: button;
  background: white;
  margin: 0 7px;
  width: 39px;
  height: 26px;
  position: absolute;
}
#renderer_form.toggler label {
  display: inline-block;
  padding: 0;
  width: 39px;
  padding-top: 27px;
  text-align: center;
  font-size: 10px;
  color: #808080;
  background-repeat: no-repeat;
  position: relative;
  margin: 0 7px;
  cursor: pointer;
}




</style>

<div class="tab-pane" id="Timeline">
  <h2>Processes Timeline</h2><br/> 
  <table class="table table-bordered">
    <tr>
      
      <th class="text-center span3">
        <legend>Legend</legend>
        <div id="legend"></div>
        <br/><br/>
        <legend>Type</legend>
        <form id="offset_form" class="toggler">
          <fieldset>
            <label class="radio inline">
              <input type="radio" name="offset" id="stack" value="zero" checked>
              Stack
            </label>
            <label class="radio inline">
              <input type="radio" name="offset" id="lines" value="lines">
              Lines
            </label>
          </fieldset>
        </form>
       
      </th>

      <td class="span3">
        <div id="chart_container">
          <div id="chart"></div>
          <div id="line"></div>
          <div id="preview"></div>
        </div>
      </td>
    
    </tr>
  </table>

<script src="/js/timeline.js"></script>
<script>
  do_timeline("/lua/get_processes_data.lua",{ mode: "timeline"},"name",2,300,2000);
</script>
</div>
]]

dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
