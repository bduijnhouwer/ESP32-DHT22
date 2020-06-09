<?php
# Loading config data from *.ini-file
$ini = parse_ini_file ('config.ini');

# Assigning the ini-values to usable variables
$db_host = $ini['db_host'];
$db_name = $ini['db_name'];
$db_table = $ini['db_table'];
$db_user = $ini['db_user'];
$db_password = $ini['db_password'];

# Prepare a connection to the mySQL database
$connection = new mysqli($db_host, $db_user, $db_password, $db_name);

?>
<!-- start of the HTML part that Google Chart needs -->
<html>
<head>
        <script type="text/javascript" src="https://www.gstatic.com/charts/loader.js"></script>
<!-- This loads the 'corechart' package. -->
    <script type="text/javascript">
        google.charts.load('current', {'packages':['corechart']});
        google.charts.setOnLoadCallback(drawChart);

                function drawChart() {
                var data = google.visualization.arrayToDataTable([
                       ['Timestamp', 'Temperature', 'Humidity'],
<?php

# This query connects to the database and get the last 10 readings
$sql = "SELECT temperature, humidity, timestamp FROM $db_table";

$result = $connection->query($sql);

# This while - loop formats and put all the retrieved data into ['timestamp', 'temperature', 'humidity'] way.
        while ($row = $result->fetch_assoc()) {
                $timestamp_rest = substr($row["timestamp"],10,6);
                echo "['".$timestamp_rest."',".$row['temperature'].",".$row['humidity']."],";
                }
?>
]);

// Curved line
var options = {
                title: 'Temperature and humidity',
                curveType: 'function',
                legend: { position: 'bottom' },
                hAxis: {
                        slantedText:true,
                        slantedTextAngle:45
                        }
                };

// Curved chart
var chart = new google.visualization.LineChart(document.getElementById('curve_chart'));
chart.draw(data, options);

} // End bracket from drawChart
//</script>

<!-- The charts below is ony available in the 'bar' package -->
<script type="text/javascript">
</script>
</head>

<?php

# Prepare a connection to the mySQL database
$connection = new mysqli($db_host, $db_user, $db_password, $db_name);

?>
<div id="curve_chart" style="width: 1600px; height: 640px;"></div>
<div id="barchart_values" style="width: 900px; height: 480px;"></div>
<div id="top_x_div" style="width: 900px; height: 480px;"></div>