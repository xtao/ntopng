--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"

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
  <link type="text/css" rel="stylesheet" href="/css/Rickshaw/extensions.css?v=2">

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

  <script src="/js/Rickshaw/extensions.js"></script>
<div class="tab-pane" id="Timeline"> 
<div id="content">

  <form id="side_panel">
    <h1>Processes Timeline</h1>
    <section><div id="legend"></div></section>
   <!-- <section>
      <div id="renderer_form" class="toggler">
        <input type="radio" name="renderer" id="area" value="area" checked>
        <label for="area">area</label>
        <input type="radio" name="renderer" id="bar" value="bar">
        <label for="bar">bar</label>
        <input type="radio" name="renderer" id="line" value="line">
        <label for="line">line</label>
        <input type="radio" name="renderer" id="scatter" value="scatterplot">
        <label for="scatter">scatter</label>
      </div>
    </section>
    <section>
      <div id="offset_form">
        <label for="stack">
          <input type="radio" name="offset" id="stack" value="zero" checked>
          <span>stack</span>
        </label>
        <label for="stream">
          <input type="radio" name="offset" id="stream" value="wiggle">
          <span>stream</span>
        </label>
        <label for="pct">
          <input type="radio" name="offset" id="pct" value="expand">
          <span>pct</span>
        </label>
        <label for="value">
          <input type="radio" name="offset" id="value" value="value">
          <span>value</span>
        </label>
      </div>
      <div id="interpolation_form">
        <label for="cardinal">
          <input type="radio" name="interpolation" id="cardinal" value="cardinal" checked>
          <span>cardinal</span>
        </label>
        <label for="linear">
          <input type="radio" name="interpolation" id="linear" value="linear">
          <span>linear</span>
        </label>
        <label for="step">
          <input type="radio" name="interpolation" id="step" value="step-after">
          <span>step</span>
        </label>
      </div>
    </section>
    <section>
      <h6>Smoothing</h6>
      <div id="smoother"></div>
    </section>
    <section></section> -->
  </form>

  <div id="chart_container">
    <div id="chart"></div>
    <div id="timeline"></div>
    <div id="preview"></div>
  </div>

</div>

<script src="/js/timeline.js"></script>
</div>
]]

dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
