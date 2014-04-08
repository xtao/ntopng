// Wrapper function
function do_timeline(p_update_url, p_url_params, p_url_id_name, p_timeInterval, p_init_period) {
  var tml = new Timeline(p_update_url, p_url_params, p_url_id_name, p_timeInterval, p_init_period);
  var refresh = (p_timeInterval*1000);
  tml.setInterval(setInterval(function(){tml.update();},refresh ));

  // Return new class instance, with
  return tml;
}


// "/lua/get_processes_data.lua",{ mode: "timeline"},"name",2,300);
function Timeline(p_update_url, p_url_params, p_url_id_name, p_timeInterval, p_init_period) {

  // set up our data series with 150 random data points
  this.n_seriesData = [];
  this.n_seriesID = [];

  this.interval;
  this.timeline_control;
  this.timeline_control = TimelineValue(this,p_update_url, p_url_params, p_url_id_name, p_timeInterval, p_init_period);
  this.annotator;

  // Initial graph structure
  var graph_info = {
    element: document.getElementById("chart"),
    width: 800,
    height: 400,
    renderer: 'area',
    stroke: false,
    preserve: true,
    series: []
  } 

  this.timeline_control.initJsonData(this.n_seriesData,this.n_seriesID,graph_info);

  for (var i = 0; i < p_init_period; i++) {
    this.timeline_control.addDataEmpty(this.n_seriesData);
  }

  // instantiate our graph!
  var graph = new Rickshaw.Graph(graph_info);
  graph.render();

  var preview = new Rickshaw.Graph.RangeSlider.Preview( {
    graph: graph,
    element: document.getElementById('preview')
  } );

  var hoverDetail = new Rickshaw.Graph.HoverDetail( {
    graph: graph,
    xFormatter: function(x) {
      return new Date(x * 1000).toString();
    }
  } );

  this.annotator = new Rickshaw.Graph.Annotate( {
    graph: graph,
    element: document.getElementById('line')
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

  var previewXAxis = new Rickshaw.Graph.Axis.Time({
    graph: preview.previews[0],
    timeFixture: new Rickshaw.Fixtures.Time.Local(),
    ticksTreatment: ticksTreatment
  });

  previewXAxis.render();

  // add some data every so often
  this.update = function () {
    this.timeline_control.removeData(this.n_seriesData);
    this.timeline_control.addJsonData(this.n_seriesData,this.n_seriesID);
    graph.update();
  }

  // var messages = [
  //   "Changed home page welcome message",
  //   "Minified JS and CSS",
  //   "Changed button color from blue to green",
  //   "Refactored SQL query to use indexed columns",
  //   "Added additional logging for debugging",
  //   "Fixed typo",
  //   "Rewrite conditional logic for clarity",
  //   "Added documentation for new methods"
  // ];

  // function addAnnotation(force) {
  //   if (messages.length > 0 && (force || Math.random() >= 0.95)) {
  //     annotator.add(seriesData[2][seriesData[2].length-1].x, messages.shift());
  //     annotator.update();
  //   }
  // }

  // addAnnotation(true);
  // setTimeout( function() { setInterval( addAnnotation, 6000 ) }, 6000 );

}

///////////////////////////////////////////////////////////
// PUBLIC FUNCIONTS ////////////////////////////////////
///////////////////////////////////////////////////////////

Timeline.prototype.setInterval = function(p_interval) {
  this.interval = p_interval;
}

Timeline.prototype.stopInterval = function() {
    //disabled graph interval
    clearInterval(this.interval);
}

Timeline.prototype.startInterval = function() {
  this.interval = setInterval(this.update(), this.refresh)
}

Timeline.prototype.addAnnotation = function(p_proc_name,p_message) {
  var index = -1;
  
  this.n_seriesID.forEach(function (name,i) {
    if(name == p_proc_name) {index = i;}
  });

  if(index != -1){
    this.n_seriesData[index]
    this.annotator.add(this.n_seriesData[index][this.n_seriesData[index].length-1].x, p_message);
  } else {
    console.log("Process doesn't found. Impossible insert messages.");
  }
}

///////////////////////////////////////////////////////////
// UPDATE CLASS ////////////////////////////////////
///////////////////////////////////////////////////////////

function TimelineValue (p_timeline,p_update_url, p_url_params, p_url_id_name, p_timeInterval, p_init_period) {

  this.update_url = p_update_url
  this.url_params = p_url_params
  this.url_id_name = p_url_id_name
  this.timeInterval = p_timeInterval || 1;
  this.timeline = p_timeline;

  var addData;
  var palette = new Rickshaw.Color.Palette( /*{ scheme: 'classic9' }*/ );
  var timeBase = Math.floor(new Date().getTime() / 1000) - (p_init_period*timeInterval);
  var n_lastValuesSeriesData = [];

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


  this.addJsonData = function(p_data,id) {

    var index = p_data[0].length;
    
    id.forEach( function (current_id,i) {
      var value = getNewValue(current_id);
      
      if (value == -1) {
        // value = n_lastValuesSeriesData[i];
        value = 0;
        this.timeline.addAnnotation(current_id,"The " + current_id + " process is inactive.");
        // console.log("getNewValue JSON empty => Process ID:"+current_id+", Old Value: "+value);
      }
      
      p_data[i].push( { x: (index * timeInterval) + timeBase, y: value } );
      n_lastValuesSeriesData[i] = value;
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
          var value = (jsonData[0] != null)? jsonData[0].value : -1 ;
          l_ret = (value);
       },
       error: function(content) {
         console.log("getNewValue JSON error");
       }
     });
      return l_ret;
    }
  
  }; //End addJsonData


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


