var flow_rows = [];
var flow_rows_index = 0;


function flow_table_setID (row) {

  var flow_key = row.find("td").eq(0).text();
  row.attr('id', flow_key);
  row.find("td").eq(0).attr('id', flow_key+"_key");
  row.find("td").eq(1).attr('id', flow_key+"_info");
  row.find("td").eq(2).attr('id', flow_key+"_application");
  row.find("td").eq(3).attr('id', flow_key+"_l4");
  row.find("td").eq(4).attr('id', flow_key+"_vlan");
  row.find("td").eq(5).attr('id', flow_key+"_client");
  row.find("td").eq(6).attr('id', flow_key+"_server");
  row.find("td").eq(7).attr('id', flow_key+"_duration");
  row.find("td").eq(8).attr('id', flow_key+"_breakdown");
  row.find("td").eq(9).attr('id', flow_key+"_throughput");
  row.find("td").eq(10).attr('id', flow_key+"_bytes");
  flow_rows[flow_rows_index] = flow_key;
  flow_rows_index++;
  // console.log(row);
  //console.log(flow_rows);
  return row;

}


function flow_table_update () {
  $.ajax({
    type: 'GET',
    url: url_update_all,
    success: function(content) {
      // console.log(jQuery.parseJSON(content));
      var rows_data = jQuery.parseJSON(content);

      for (var row in rows_data["data"]) {
        //console.log(rows_data.data[row].key);
        var flow_key = rows_data.data[row].key.toString();

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

var flow_table_interval = window.setInterval(flow_table_update,2000);
