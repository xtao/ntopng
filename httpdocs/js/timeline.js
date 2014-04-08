
// set up our data series with 150 random data points

var seriesData = [ [], [], [], [], [], [], [], [], [] ];
var seriesID = [];

var n_seriesData = [];
var n_seriesID = [];
var graph_info = {
  element: document.getElementById("chart"),
  width: 800,
  height: 400,
  renderer: 'area',
  stroke: false,
  preserve: true,
  series: []
} 
var palette = new Rickshaw.Color.Palette( /*{ scheme: 'classic9' }*/ );
var timeline_control = TimelineValue("/lua/get_processes_data.lua",{ mode: "timeline"},"name",1,600);

timeline_control.initJsonData(n_seriesData,n_seriesID,graph_info);

for (var i = 0; i < 600; i++) {
  timeline_control.addDataEmpty(n_seriesData);
}


// instantiate our graph!

var graph = new Rickshaw.Graph(graph_info);

graph.render();

var preview = new Rickshaw.Graph.RangeSlider.Preview( {
  graph: graph,
  element: document.getElementById('preview'),
} );

var hoverDetail = new Rickshaw.Graph.HoverDetail( {
  graph: graph,
  xFormatter: function(x) {
    return new Date(x * 1000).toString();
  }
} );

var annotator = new Rickshaw.Graph.Annotate( {
  graph: graph,
  element: document.getElementById('timeline')
} );

var legend = new Rickshaw.Graph.Legend( {
  graph: graph,
  element: document.getElementById('legend')

} );

var shelving = new Rickshaw.Graph.Behavior.Series.Toggle( {
  graph: graph,
  legend: legend
} );

var order = new Rickshaw.Graph.Behavior.Series.Order( {
  graph: graph,
  legend: legend
} );

var highlighter = new Rickshaw.Graph.Behavior.Series.Highlight( {
  graph: graph,
  legend: legend
} );

var smoother = new Rickshaw.Graph.Smoother( {
  graph: graph,
  element: $('#smoother')
} );

var ticksTreatment = 'glow';

var xAxis = new Rickshaw.Graph.Axis.Time( {
  graph: graph,
  ticksTreatment: ticksTreatment,
  timeFixture: new Rickshaw.Fixtures.Time.Local()
} );

xAxis.render();

var yAxis = new Rickshaw.Graph.Axis.Y( {
  graph: graph,
  tickFormat: Rickshaw.Fixtures.Number.formatKMBT,
  ticksTreatment: ticksTreatment
} );

yAxis.render();


var controls = new RenderControls( {
  element: document.querySelector('form'),
  graph: graph
} );

// add some data every so often

var messages = [
  "Changed home page welcome message",
  "Minified JS and CSS",
  "Changed button color from blue to green",
  "Refactored SQL query to use indexed columns",
  "Added additional logging for debugging",
  "Fixed typo",
  "Rewrite conditional logic for clarity",
  "Added documentation for new methods"
];

setInterval( function() {
  timeline_control.removeData(n_seriesData);
  timeline_control.addJsonData(n_seriesData,n_seriesID);
  graph.update();

}, 1000 );

function addAnnotation(force) {
  if (messages.length > 0 && (force || Math.random() >= 0.95)) {
    annotator.add(seriesData[2][seriesData[2].length-1].x, messages.shift());
    annotator.update();
  }
}

// addAnnotation(true);
// setTimeout( function() { setInterval( addAnnotation, 6000 ) }, 6000 );

var previewXAxis = new Rickshaw.Graph.Axis.Time({
  graph: preview.previews[0],
  timeFixture: new Rickshaw.Fixtures.Time.Local(),
  ticksTreatment: ticksTreatment
});

previewXAxis.render();


function TimelineValue (p_update_url, p_url_params, p_url_id_name, p_timeInterval, p_init_period) {

  var addData;
  
  this.update_url = p_update_url
  this.url_params = p_url_params
  this.url_id_name = p_url_id_name
  this.timeInterval = p_timeInterval || 1;

  var timeBase = Math.floor(new Date().getTime() / 1000) - p_init_period;

  this.initJsonData = function(p_data,p_id,p_graph_info) {

    var index = 0;

    $.ajax({
       type: 'GET',
       url: this.update_url,
       data: this.url_params,
       async: false,
       success: function(content) {
          var jsonData = jQuery.parseJSON(content);
          
          jsonData.forEach( function (flow,i) {
            p_data[i] = [];
            p_data[i].push( { x: (index * timeInterval) + timeBase, y: flow.value } );
            p_graph_info.series.push(
            {
             color: palette.color(),
             data: p_data[i],
             name: flow.label
            });

            p_id.push(flow.name);
          });
       },
       error: function(content) {
         console.log("initJsonData JSON error");
       }
     });
  
  }; //End initJsonData


  this.addJsonData = function(data,id) {

    var index = data[0].length;
    
    id.forEach( function (current_id,i) {
      data[i].push( { x: (index * timeInterval) + timeBase, y: getNewValue(current_id) } );
    });

    function getNewValue(p_current_id) {
      var l_ret = 0;
      var l_url_param = this.url_params

      l_url_param[url_id_name] = p_current_id

      $.ajax({
       type: 'GET',
       url: this.update_url,
       data: l_url_param,
       async: false,
       success: function(content) {
          var jsonData = jQuery.parseJSON(content);
          var value = jsonData[0].value;
          if (value == null) {value =1;}
          l_ret = (value);
       },
       error: function(content) {
         console.log("getNewValue JSON error");
       }
     });
      return l_ret;
    }
  
  }; //End addJsonData

  this.addData = function(p_data) {

    var randomValue = Math.random() * 100 + 15 + lastRandomValue;
    var index = p_data[0].length;
    // alert(index);

    var counter = 1;

    p_data.forEach( function(series) {
      var randomVariance = Math.random() * 20;
      var v = randomValue / 25  + counter++ +
        (Math.cos((index * counter * 11) / 960) + 2) * 15 +
        (Math.cos(index / 7) + 2) * 7 +
        (Math.cos(index / 17) + 2) * 1;

      series.push( { x: (index * timeInterval) + timeBase, y: v + randomVariance } );
    } );

    lastRandomValue = randomValue * 0.85;
  };


  this.addDataEmpty = function(p_data) {

    var index = p_data[0].length;
    // alert(index);

    p_data.forEach( function(series) {
      series.push( { x: (index * timeInterval) + timeBase, y: 0 } );
    } );

  };


  this.removeData = function(data) {
    data.forEach( function(series) {
      series.shift();
    } );
    timeBase += timeInterval;
  };

  return this;
};
