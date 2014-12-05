

// ---------------- Automatic AS table update code ------------------------
// Use the host_rows_option object in order to simplify the option setting from lua script.

function host_table_setID (row) {
  var index = 0;
  var as_key = row.find("td").eq(0).text();
  console.log(as_key);

  // Set the row index to the host key
  row.attr('id', as_key);

  row.find("td").eq(index++).attr('id', as_key+"_key");
  // hosts_stats_top
  row.find("td").eq(index++).attr('id', as_key+"_alerts");
  row.find("td").eq(index++).attr('id', as_key+"_name");
  row.find("td").eq(index++).attr('id', as_key+"_since");

  // hosts_stats_bottom
  row.find("td").eq(index++).attr('id', as_key+"_breakdown");
  row.find("td").eq(index++).attr('id', as_key+"_throughput");
  row.find("td").eq(index++).attr('id', as_key+"_traffic");

  console.log(row);
  return row;

}

function row_update(as_key) {
   var asInfo = as_key;
   console.log("row_update");
   var url = "@HTTP_PREFIX@/lua/get_host_data.lua?host="+asInfo[0];

  $.ajax({
    type: 'GET',
    url: url,
    cache: false,
    success: function(content) {
      var data = jQuery.parseJSON(content);
      // console.log(url);
      // console.log(data);
      $("#"+as_key+'_since').html(data.column_since);
      $("#"+as_key+'_breakdown').html(data.column_breakdown);
      $("#"+as_key+'_throughput').html(data.column_thpt);
      $("#"+as_key+'_traffic').html(data.column_traffic);
    },
    error: function(content) {
      console.log("error");
    }
  });
}

// Updating function
function as_table_update () {

  var $dt = $("#table-as").data("datatable");
  var rows = $dt.rows;
  console.log(rows);

  for (var row in rows){
    var as_key = rows[row][0].id;
    row_update(as_key);
  }
}

// Refresh Interval (10 sec)
var as_table_interval = window.setInterval(as_table_update, 10000);
// ---------------- End automatic table update code ------------------------
