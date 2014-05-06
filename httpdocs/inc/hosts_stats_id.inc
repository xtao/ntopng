// Use the host_rows_option object in order to simplify the option setting from lua script.

var host_rows_option = {};
host_rows_option["ip"] = false;
host_rows_option["categorization"] = false;
host_rows_option["httpbl"] = false;

var host_rows = [];
var host_rows_index = 0;


function host_table_setID (row) {
  var index = 0;
  var host_key = row.find("td").eq(0).text();
  
  // Set the row index to the host key
  row.attr('id', host_key);

    
  
  row.find("td").eq(index++).attr('id', host_key+"_key");
  //custom
  if (host_rows_option["ip"]) row.find("td").eq(index++).attr('id', host_key+"_ip");
  row.find("td").eq(index++).attr('id', host_key+"_vlan");
  // hosts_stats_top
  row.find("td").eq(index++).attr('id', host_key+"_location");
  row.find("td").eq(index++).attr('id', host_key+"_alerts");
  row.find("td").eq(index++).attr('id', host_key+"_name");
  row.find("td").eq(index++).attr('id', host_key+"_since");
  row.find("td").eq(index++).attr('id', host_key+"_asn");
  
  //custom
  if (host_rows_option["categorization"]) row.find("td").eq(index++).attr('id', host_key+"_category");
  if (host_rows_option["httpbl"]) row.find("td").eq(index++).attr('id', host_key+"_httpbl");
  
  // hosts_stats_bottom
  row.find("td").eq(index++).attr('id', host_key+"_breakdown");
  row.find("td").eq(index++).attr('id', host_key+"_throughput");
  row.find("td").eq(index++).attr('id', host_key+"_traffic");

  host_rows[host_rows_index] = host_key;
  host_rows_index++;
  
  // console.log(row);
  // console.log(host_rows);
  return row;

}

// Updating function
function host_table_update () {
  $.ajax({
    type: 'GET',
    url: url_update_all,
    success: function(content) {
      // console.log(jQuery.parseJSON(content));
      var rows_data = jQuery.parseJSON(content);

      for (var row in rows_data["data"]) {
        //console.log(rows_data.data[row].key);
        var host_key = rows_data.data[row].key;

        if (host_rows.indexOf(host_key) != -1) {
          // console.log("#"+host_key+'_traffic -->' + $("#"+host_key+'_traffic').html());
          $("#"+host_key+'_since').html(rows_data.data[row].column_since);
          $("#"+host_key+'_breakdown').html(rows_data.data[row].column_breakdown);
          $("#"+host_key+'_throughput').html(rows_data.data[row].column_thpt);
          $("#"+host_key+'_traffic').html(rows_data.data[row].column_traffic);
        } else {
        //console.log('New host: '+host_key);
        //$("#table-hosts tr#"+host_key).remove()
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
var host_table_interval = window.setInterval(host_table_update,1000);
