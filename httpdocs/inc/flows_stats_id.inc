// Use the flow_rows_option object in order to simplify the option setting from lua script.

var flow_rows_option = {};
flow_rows_option["type"] = "flow";
flow_rows_option["categorization"] = false;
flow_rows_option["sprobe"] = false;

var flow_rows = [];
var flow_rows_index = 0;


function flow_table_setID (row) {
  var index = 0;
  var flow_key = row.find("td").eq(0).text();
  var type = flow_rows_option["type"];
  // console.log(type);
  
  // Set the row index to the flow key
  row.attr('id', flow_key);

  if (!flow_rows_option["sprobe"]) {
    
    // flows_stats_top
    if ((type == 'flow') || (type == 'host')) row.find("td").eq(index++).attr('id', flow_key+"_key");
    if ((type == 'flow') || (type == 'host')) row.find("td").eq(index++).attr('id', flow_key+"_info");
    if ((type == 'flow') || (type == 'host')) row.find("td").eq(index++).attr('id', flow_key+"_application");
    if ((type == 'flow') || (type == 'host')) row.find("td").eq(index++).attr('id', flow_key+"_l4");
    if ((type == 'flow') || (type == 'host')) row.find("td").eq(index++).attr('id', flow_key+"_vlan");
    if ((type == 'flow') || (type == 'host')) row.find("td").eq(index++).attr('id', flow_key+"_client");
    if ((type == 'flow') || (type == 'host')) row.find("td").eq(index++).attr('id', flow_key+"_server");
    if ((type == 'flow') || (type == 'host')) row.find("td").eq(index++).attr('id', flow_key+"_duration");
    //custom
    if ((type == 'flow') && (flow_rows_option["categorization"])) row.find("td").eq(index++).attr('id', flow_key+"_category");
    // flows_stats_bottom
    if (type == 'flow') row.find("td").eq(index++).attr('id', flow_key+"_breakdown");
    if ((type == 'flow') || (type == 'host')) row.find("td").eq(index++).attr('id', flow_key+"_throughput");
    if ((type == 'flow') || (type == 'host')) row.find("td").eq(index++).attr('id', flow_key+"_bytes");
  
  } else {

    // flows_stats_top
    if ((type == 'flow') || (type == 'host')) row.find("td").eq(index++).attr('id', flow_key+"_key");
    if ((type == 'flow') || (type == 'host')) row.find("td").eq(index++).attr('id', flow_key+"_info");
    if ((type == 'flow') || (type == 'host')) row.find("td").eq(index++).attr('id', flow_key+"_cliproc");
    if ((type == 'flow') || (type == 'host')) row.find("td").eq(index++).attr('id', flow_key+"_srvproc");
    if ((type == 'flow') || (type == 'host')) row.find("td").eq(index++).attr('id', flow_key+"_application");
    if ((type == 'flow') || (type == 'host')) row.find("td").eq(index++).attr('id', flow_key+"_l4");
    if ((type == 'flow') || (type == 'host')) row.find("td").eq(index++).attr('id', flow_key+"_client");
    if ((type == 'flow') || (type == 'host')) row.find("td").eq(index++).attr('id', flow_key+"_server");
    if ((type == 'flow') || (type == 'host')) row.find("td").eq(index++).attr('id', flow_key+"_duration");
    //custom
    
    // flows_stats_bottom
    if (type == 'flow') row.find("td").eq(index++).attr('id', flow_key+"_breakdown");
    if ((type == 'flow') || (type == 'host')) row.find("td").eq(index++).attr('id', flow_key+"_bytes");

  }

  flow_rows[flow_rows_index] = flow_key;
  flow_rows_index++;
  
  //console.log(row);
  //console.log(flow_rows);
  return row;

}

// Updating function
function flow_table_update () {
  $.ajax({
    type: 'GET',
    url: url_update_all,
    success: function(content) {
      // console.log(jQuery.parseJSON(content));
      var rows_data = jQuery.parseJSON(content);

      for (var row in rows_data["data"]) {
        //console.log(rows_data.data[row].key);
        var flow_key = rows_data.data[row].key;

        if (flow_rows.indexOf(flow_key) != -1) {
          // console.log('OK '+flow_key);
          $("#"+flow_key+'_duration').html(rows_data.data[row].column_duration);
          $("#"+flow_key+'_breakdown').html(rows_data.data[row].column_breakdown);
          $("#"+flow_key+'_throughput').html(rows_data.data[row].column_thpt);
          $("#"+flow_key+'_bytes').html(rows_data.data[row].column_bytes);
        } else {
        //console.log('New flow: '+flow_key);
        //$("#table-hosts tr#"+flow_key).remove()
        //$('#table-hosts').find('table').append('<tr><td>my data</td><td>more data</td></tr>');
      }
      }

    },
    error: function(content) {
      console.log("error");
    }
  });
}

// Interval
// var flow_table_interval = window.setInterval(flow_table_update,10000);
