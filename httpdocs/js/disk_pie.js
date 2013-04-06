var disk_array_size = 2; /* used, free */
var disk_lines, disk_value_labels, disk_name_labels;
var disk_pie_data = [];       
var disk_old_pie_data = [];
var disk_filtered_pie_data = [];

var disk_donut = d3.layout.pie()
  .sort(null)
  .value(function(d){
      return d.item_value;
  });

var disk_color = d3.scale.category20();

var disk_arc = d3.svg.arc()
    .startAngle(function(d){ return d.startAngle; })
    .endAngle(function(d){ return d.endAngle; })
    .innerRadius(0 /* pie_ir */)
    .outerRadius(pie_r);

var disk_streaker_data_added;

function fill_disk_array(value, index, array) {
  switch(index) {
    case 0: return { item_name: "used", item_value: disk_used };
    case 1: return { item_name: "free", item_value: disk_free };
    default: return { item_name: "unknown", item_value: 0 };
  }
}

var disk_vis = d3.select("#disk-pie-chart").append("svg:svg")
    .attr("width", pie_w)
    .attr("height", pie_h);

var disk_arc_group = disk_vis.append("svg:g")
    .attr("class", "arc")
    .attr("transform", "translate(" + (pie_w/2) + "," + (pie_h/2) + ")");

var disk_label_group = disk_vis.append("svg:g")
    .attr("class", "label_group")
    .attr("transform", "translate(" + (pie_w/2) + "," + (pie_h/2) + ")");

var disk_center_group = disk_vis.append("svg:g")
    .attr("class", "center_group")
    .attr("transform", "translate(" + (pie_w/2) + "," + (pie_h/2) + ")");

var disk_paths = disk_arc_group.append("svg:circle")
       .attr("fill", "#EFEFEF")
       .attr("r", pie_r);

var disk_white_circle = disk_center_group.append("svg:circle")
    .attr("fill", "white")
    .attr("r", 0 /* pie_ir */);

var disk_total_label = disk_center_group.append("svg:text")
    .attr("class", "label")
    .attr("dy", -15)
    .attr("text-anchor", "middle")
    .text("TOTAL");

var disk_total_value = disk_center_group.append("svg:text")
    .attr("class", "total")
    .attr("dy", 7)
    .attr("text-anchor", "middle")
    .text("Waiting..");

var disk_total_units = disk_center_group.append("svg:text")
    .attr("class", "units")
    .attr("dy", 21)
    .attr("text-anchor", "middle")
    .text("GB");

function disk_update() {
    disk_streaker_data_added = d3.range(disk_array_size).map(fill_disk_array);

    disk_old_pie_data = disk_filtered_pie_data;
    disk_pie_data = disk_donut(disk_streaker_data_added);

    var totalOctets = 0;
    disk_filtered_pie_data = disk_pie_data.filter(filterData);
    function filterData(element, index, array) {
       element.name = disk_streaker_data_added[index].item_name;
       element.value = disk_streaker_data_added[index].item_value;
       totalOctets += element.value;
       return (element.value > 0);
    }

    if(disk_filtered_pie_data.length > 0 && disk_old_pie_data.length > 0){
       disk_arc_group.selectAll("circle").remove();

       disk_total_value.text(function(){
          var kb = totalOctets/1024;
          return kb.toFixed(1);
       });

       /* draw arc paths */
       disk_paths = disk_arc_group.selectAll("path").data(disk_filtered_pie_data);
       disk_paths.enter().append("svg:path")
          .attr("stroke", "white")
          .attr("stroke-width", 0.5)
          .attr("fill", function(d, i) { return disk_color(i); })
          .transition()
             .duration(pie_tween_duration)
             .attrTween("d", disk_pie_tween);
       disk_paths
          .transition()
             .duration(pie_tween_duration)
             .attrTween("d", disk_pie_tween);
       disk_paths.exit()
          .transition()
             .duration(pie_tween_duration)
             .attrTween("d", disk_remove_pie_tween)
          .remove();

       /* draw mark lines for labels */
       disk_lines = disk_label_group.selectAll("line").data(disk_filtered_pie_data);
       disk_lines.enter().append("svg:line")
          .attr("x1", 0)
          .attr("x2", 0)
          .attr("y1", -pie_r-3)
          .attr("y2", -pie_r-8)
          .attr("stroke", "gray")
          .attr("transform", function(d) {
             return "rotate(" + (d.startAngle+d.endAngle)/2 * (180/Math.PI) + ")";
          });
       disk_lines.transition()
          .duration(pie_tween_duration)
          .attr("transform", function(d) {
             return "rotate(" + (d.startAngle+d.endAngle)/2 * (180/Math.PI) + ")";
          });
       disk_lines.exit().remove();

       /* draw labels with percentage */
       disk_value_labels = disk_label_group.selectAll("text.value").data(disk_filtered_pie_data)
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

       disk_value_labels.enter().append("svg:text")
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

       disk_value_labels.transition().duration(pie_tween_duration).attrTween("transform", disk_text_tween);

       disk_value_labels.exit().remove();

       /* draw labels with names */
       disk_name_labels = disk_label_group.selectAll("text.units").data(disk_filtered_pie_data)
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

       disk_name_labels.enter().append("svg:text")
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

       disk_name_labels.transition().duration(pie_tween_duration).attrTween("transform", disk_text_tween);

       disk_name_labels.exit().remove();
    }    
}

/* arcs interpolation */
function disk_pie_tween(d, i) {
    var s0;
    var e0;
    if(disk_old_pie_data[i]){
       s0 = disk_old_pie_data[i].startAngle;
       e0 = disk_old_pie_data[i].endAngle;
    } else if (!(disk_old_pie_data[i]) && disk_old_pie_data[i-1]) {
       s0 = disk_old_pie_data[i-1].endAngle;
       e0 = disk_old_pie_data[i-1].endAngle;
    } else if(!(disk_old_pie_data[i-1]) && disk_old_pie_data.length > 0){
       s0 = disk_old_pie_data[disk_old_pie_data.length-1].endAngle;
       e0 = disk_old_pie_data[disk_old_pie_data.length-1].endAngle;
    } else {
       s0 = 0;
       e0 = 0;
    }
    var i = d3.interpolate({startAngle: s0, endAngle: e0}, {startAngle: d.startAngle, endAngle: d.endAngle});
    return function(t) {
       var b = i(t);
       return disk_arc(b);
    };
}

function disk_remove_pie_tween(d, i) {
    s0 = 2 * Math.PI;
    e0 = 2 * Math.PI;
    var i = d3.interpolate({startAngle: d.startAngle, endAngle: d.endAngle}, {startAngle: s0, endAngle: e0});
    return function(t) {
       var b = i(t);
       return disk_arc(b);
    };
}

function disk_text_tween(d, i) {
    var a;
    if(disk_old_pie_data[i]){
       a = (disk_old_pie_data[i].startAngle + disk_old_pie_data[i].endAngle - Math.PI)/2;
    } else if (!(disk_old_pie_data[i]) && disk_old_pie_data[i-1]) {
       a = (disk_old_pie_data[i-1].startAngle + disk_old_pie_data[i-1].endAngle - Math.PI)/2;
    } else if(!(disk_old_pie_data[i-1]) && disk_old_pie_data.length > 0) {
       a = (disk_old_pie_data[disk_old_pie_data.length-1].startAngle + disk_old_pie_data[disk_old_pie_data.length-1].endAngle - Math.PI)/2;
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
