

// ---------------- Automatic table update code ------------------------
// Use the processes_rows_option object in order to simplify the option setting from lua script.

var processes_rows_option = {};
processes_rows_option["source_id"] = false;

function processes_table_setID (row) {
  var index = 0;
  var process_key = row.find("td").eq(0).text();
  
  // Set the row index to the flow key
  row.attr('id', process_key);

  // processes_stats
  row.find("td").eq(index++).attr('id', process_key+"_key");
  row.find("td").eq(index++).attr('id', process_key+"_name");
  if (processes_rows_option["source_id"]) row.find("td").eq(index++).attr('id', process_key+"_vlan");
  row.find("td").eq(index++).attr('id', process_key+"_count");
  row.find("td").eq(index++).attr('id', process_key+"_duration");
  row.find("td").eq(index++).attr('id', process_key+"_bytes_sent");
  row.find("td").eq(index++).attr('id', process_key+"_bytes_rcvd");
  
  //console.log(row);
  return row;

}


function row_update(process_key) {
   var url = "/lua/get_processes_data.lua?name="+process_key;

  $.ajax({
    type: 'GET',
    url: url,
    cache: false,
    success: function(content) {
      var response = jQuery.parseJSON(content);
      var data = response.data[0]
      // console.log(url);
      // console.log(data);

      $("#"+process_key+'_duration').html(data.column_duration);
      $("#"+process_key+'_count').html(data.column_count);
      $("#"+process_key+'_bytes_sent').html(data.column_bytes_sent);
      $("#"+process_key+'_bytes_rcvd').html(data.column_bytes_rcvd);

    },
    error: function(content) {
      console.log("error");
    }
  });
}

// Updating function
function processes_table_update () {

  var $dt = $("#table-processes").data("datatable"); 
  // var currentPage = $dt.options.currentPage;
  // var perPage = $dt.options.perPage;
  var rows = $dt.rows;

  for (var row in rows){
    var process_key = rows[row][0].id;
    row_update(process_key);
  }
}

// Refresh Interval (10 sec)
var processes_table_interval = window.setInterval(processes_table_update, 1000);
// ---------------- End automatic table update code ------------------------


