<?php
ini_set('display_errors',1);
require_once 'php-activerecord/ActiveRecord.php';
ActiveRecord\Config::initialize(function($cfg) {
    $cfg->set_model_directory('models');
    $cfg->set_connections(array('development' => 'pgsql://postgresql.cs.wisc.edu/cs564_patel'));
});

//Load file
$str = file_get_contents("p.csv");
$lines = explode("\n", $str);
unset($lines[0]);
foreach ($lines as $line) {
    if (trim($line) != '') {
        $popestLine = explode(",", $line);
        $popest = PopEst::create(array(
            "state" => $popestLine[0],
            "region" => $popestLine[1],
            "division" => $popestLine[2],
            "name" => $popestLine[3],
            "popestimate2010" => $popestLine[4],
            "popestimate2011" => $popestLine[5],
            "huest_2010" => $popestLine[6],
            "huest_2011" => $popestLine[7]
        ));
    }
}
?>
