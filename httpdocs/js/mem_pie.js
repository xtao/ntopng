var mem_array_size = 3; /* used, free, cached */
var mem_lines, mem_value_labels, mem_name_labels;
var mem_pie_data = [];       
var mem_old_pie_data = [];
var mem_filtered_pie_data = [];

var mem_donut = d3.layout.pie()
  .sort(null)
  .value(function(d){
      return d.item_value;
  });

var mem_color = d3.scale.category20();

var mem_arc = d3.svg.arc()
    .startAngle(function(d){ return d.startAngle; })
    .endAngle(function(d){ return d.endAngle; })
    .innerRadius(pie_ir)
    .outerRadius(pie_r);

var mem_streaker_data_added;

function fill_mem_array(value, index, array) {
  switch(index) {
    case 0: return { item_name: "used", item_value: mem_used };
	case 1: return { item_name: "free", item_value: mem_free };
	case 2: return { item_name: "cached", item_value: mem_cached };
	default: return { item_name: "unknown", item_value: 0 };
  }
}

var mem_vis = d3.select("#mem-pie-chart").append("svg:svg")
    .attr("width", pie_w)
    .attr("height", pie_h);

var mem_arc_group = mem_vis.append("svg:g")
    .attr("class", "arc")
    .attr("transform", "translate(" + (pie_w/2) + "," + (pie_h/2) + ")");

var mem_label_group = mem_vis.append("svg:g")
    .attr("class", "label_group")
    .attr("transform", "translate(" + (pie_w/2) + "," + (pie_h/2) + ")");

var mem_center_group = mem_vis.append("svg:g")
    .attr("class", "center_group")
    .attr("transform", "translate(" + (pie_w/2) + "," + (pie_h/2) + ")");

var mem_paths = mem_arc_group.append("svg:circle")
       .attr("fill", "#EFEFEF")
       .attr("r", pie_r);

var mem_white_circle = mem_center_group.append("svg:circle")
    .attr("fill", "white")
    .attr("r", pie_ir);

var mem_total_label = mem_center_group.append("svg:text")
    .attr("class", "label")
    .attr("dy", -15)
    .attr("text-anchor", "middle")
    .text("TOTAL");

var mem_total_value = mem_center_group.append("svg:text")
    .attr("class", "total")
    .attr("dy", 7)
    .attr("text-anchor", "middle")
    .text("Waiting..");

var mem_total_units = mem_center_group.append("svg:text")
    .attr("class", "units")
    .attr("dy", 21)
    .attr("text-anchor", "middle")
    .text("MB");

function mem_update() {
    mem_streaker_data_added = d3.range(mem_array_size).map(fill_mem_array);

    mem_old_pie_data = mem_filtered_pie_data;
    mem_pie_data = mem_donut(mem_streaker_data_added);

    var totalOctets = 0;
    mem_filtered_pie_data = mem_pie_data.filter(filterData);
    function filterData(element, index, array) {
       element.name = mem_streaker_data_added[index].item_name;
       element.value = mem_streaker_data_added[index].item_value;
       totalOctets += element.value;
       return (element.value > 0);
    }

    if(mem_filtered_pie_data.length > 0 && mem_old_pie_data.length > 0){
       mem_arc_group.selectAll("circle").remove();

       mem_total_value.text(function(){
          var kb = totalOctets/1024;
          return kb.toFixed(1);
       });

       /* draw arc paths */
       mem_paths = mem_arc_group.selectAll("path").data(mem_filtered_pie_data);
       mem_paths.enter().append("svg:path")
          .attr("stroke", "white")
          .attr("stroke-width", 0.5)
          .attr("fill", function(d, i) { return mem_color(i); })
          .transition()
             .duration(pie_tween_duration)
             .attrTween("d", mem_pie_tween);
       mem_paths
          .transition()
             .duration(pie_tween_duration)
             .attrTween("d", mem_pie_tween);
       mem_paths.exit()
          .transition()
             .duration(pie_tween_duration)
             .attrTween("d", mem_remove_pie_tween)
          .remove();

       /* draw mark lines for labels */
       mem_lines = mem_label_group.selectAll("line").data(mem_filtered_pie_data);
       mem_lines.enter().append("svg:line")
          .attr("x1", 0)
          .attr("x2", 0)
          .attr("y1", -pie_r-3)
          .attr("y2", -pie_r-8)
          .attr("stroke", "gray")
          .attr("transform", function(d) {
             return "rotate(" + (d.startAngle+d.endAngle)/2 * (180/Math.PI) + ")";
          });
       mem_lines.transition()
          .duration(pie_tween_duration)
          .attr("transform", function(d) {
             return "rotate(" + (d.startAngle+d.endAngle)/2 * (180/Math.PI) + ")";
          });
       mem_lines.exit().remove();

       /* draw labels with percentage */
       mem_value_labels = mem_label_group.selectAll("text.value").data(mem_filtered_pie_data)
          .attr("dy", function(d){
             if ((d.startAngle+d.endAngle)/2 > Math.PI/2 && (d.startAngle+d.endAngle)/2 < Math.PI*1.5 ) {
                return 5;
             } else {
                return -7;
             }
          })
          .attr("text-anchor", function(d){
             if ( (d.startAngle+d.endAngle)/2 < Math.PI ){
                return "beginning";
             } else {
                return "end";
             }
          })
          .text(function(d){
             var percentage = (d.value/totalOctets)*100;
             return percentage.toFixed(1) + "%";
          });

       mem_value_labels.enter().append("svg:text")
          .attr("class", "value")
          .attr("transform", function(d) {
             return "translate(" + Math.cos(((d.startAngle+d.endAngle - Math.PI)/2)) * (pie_r+pie_text_offset) + "," + Math.sin((d.startAngle+d.endAngle - Math.PI)/2) * (pie_r+pie_text_offset) + ")";
          })
          .attr("dy", function(d){
             if ((d.startAngle+d.endAngle)/2 > Math.PI/2 && (d.startAngle+d.endAngle)/2 < Math.PI*1.5 ) {
                return 5;
             } else {
                return -7;
             }
          })
          .attr("text-anchor", function(d){
             if ( (d.startAngle+d.endAngle)/2 < Math.PI ){
                return "beginning";
             } else {
                return "end";
             }
          }).text(function(d){
             var percentage = (d.value/totalOctets)*100;
             return percentage.toFixed(1) + "%";
          });

       mem_value_labels.transition().duration(pie_tween_duration).attrTween("transform", mem_text_tween);

       mem_value_labels.exit().remove();

       /* draw labels with names */
       mem_name_labels = mem_label_group.selectAll("text.units").data(mem_filtered_pie_data)
          .attr("dy", function(d){
             if ((d.startAngle+d.endAngle)/2 > Math.PI/2 && (d.startAngle+d.endAngle)/2 < Math.PI*1.5 ) {
                return 17;
             } else {
                return 5;
             }
          })
          .attr("text-anchor", function(d){
             if ((d.startAngle+d.endAngle)/2 < Math.PI ) {
                return "beginning";
             } else {
                return "end";
             }
          }).text(function(d){
             return d.name;
          });

       mem_name_labels.enter().append("svg:text")
          .attr("class", "units")
          .attr("transform", function(d) {
             return "translate(" + Math.cos(((d.startAngle+d.endAngle - Math.PI)/2)) * (pie_r+pie_text_offset) + "," + Math.sin((d.startAngle+d.endAngle - Math.PI)/2) * (pie_r+pie_text_offset) + ")";
          })
          .attr("dy", function(d){
             if ((d.startAngle+d.endAngle)/2 > Math.PI/2 && (d.startAngle+d.endAngle)/2 < Math.PI*1.5 ) {
                return 17;
             } else {
                return 5;
             }
          })
          .attr("text-anchor", function(d){
             if ((d.startAngle+d.endAngle)/2 < Math.PI ) {
                return "beginning";
             } else {
                return "end";
             }
          }).text(function(d){
             return d.name;
          });

       mem_name_labels.transition().duration(pie_tween_duration).attrTween("transform", mem_text_tween);

       mem_name_labels.exit().remove();
    }    
}

/* arcs interpolation */
function mem_pie_tween(d, i) {
    var s0;
    var e0;
    if(mem_old_pie_data[i]){
       s0 = mem_old_pie_data[i].startAngle;
       e0 = mem_old_pie_data[i].endAngle;
    } else if (!(mem_old_pie_data[i]) && mem_old_pie_data[i-1]) {
       s0 = mem_old_pie_data[i-1].endAngle;
       e0 = mem_old_pie_data[i-1].endAngle;
    } else if(!(mem_old_pie_data[i-1]) && mem_old_pie_data.length > 0){
       s0 = mem_old_pie_data[mem_old_pie_data.length-1].endAngle;
       e0 = mem_old_pie_data[mem_old_pie_data.length-1].endAngle;
    } else {
       s0 = 0;
       e0 = 0;
    }
    var i = d3.interpolate({startAngle: s0, endAngle: e0}, {startAngle: d.startAngle, endAngle: d.endAngle});
    return function(t) {
       var b = i(t);
       return mem_arc(b);
    };
}

function mem_remove_pie_tween(d, i) {
    s0 = 2 * Math.PI;
    e0 = 2 * Math.PI;
    var i = d3.interpolate({startAngle: d.startAngle, endAngle: d.endAngle}, {startAngle: s0, endAngle: e0});
    return function(t) {
       var b = i(t);
       return mem_arc(b);
    };
}

function mem_text_tween(d, i) {
    var a;
    if(mem_old_pie_data[i]){
       a = (mem_old_pie_data[i].startAngle + mem_old_pie_data[i].endAngle - Math.PI)/2;
    } else if (!(mem_old_pie_data[i]) && mem_old_pie_data[i-1]) {
       a = (mem_old_pie_data[i-1].startAngle + mem_old_pie_data[i-1].endAngle - Math.PI)/2;
    } else if(!(mem_old_pie_data[i-1]) && mem_old_pie_data.length > 0) {
       a = (mem_old_pie_data[mem_old_pie_data.length-1].startAngle + mem_old_pie_data[mem_old_pie_data.length-1].endAngle - Math.PI)/2;
    } else {
       a = 0;
    }
    var b = (d.startAngle + d.endAngle - Math.PI)/2;
    var fn = d3.interpolateNumber(a, b);
    return function(t) {
       var val = fn(t);
       return "translate(" + Math.cos(val) * (pie_r+pie_text_offset) + "," + Math.sin(val) * (pie_r+pie_text_offset) + ")";
    };
}
