<html>
<body>
<?php
include("header.php");
    $a = session_id();
    if(empty($a)) session_start();
   $db_handle = pg_connect('dbname=cs564_patel host=postgres.cs.wisc.edu sslmode=require');
   $query = "select max(popestimate2010), min(popestimate2010), max(popestimate2011), min(popestimate2011), max(huest_2010), min(huest_2010), max(huest_2011), min(huest_2011) from gooi.pop_housing_estimate_state;";
   $result = pg_exec($db_handle, $query);
   if(!$result)
   {
      echo "error with query: ".pg_errormessage($db_handle);
      die();
   }
   
   // backup variables into session
   if(isset($_POST["SearchBtn"])){
       //special case for state, region and division
       if(strlen($_POST["state_keywords"])>0) $state_name = $_POST["state_keywords"];
       if(strlen($_POST["region_keywords"])>0) $region = $_POST["region_keywords"];
       if(strlen($_POST["div_keywords"])>0) $division = $_POST["div_keywords"];

	// backup all the variables
       $_SESSION["backup"] = array($state_name, $region, $division, $_POST["min_popest2010"], $_POST["max_popest2010"],$_POST["min_popest2011"],$_POST["max_popest2011"],$_POST["min_huest2010"],$_POST["max_huest2010"], $_POST["min_huest2011"], $_POST["max_huest2011"] );
       session_write_close();

	// generate query for search
   	 $str = "SELECT * FROM gooi.pop_housing_estimate_state where ";
   	 if(!strlen($_POST['state_keywords'])==0){
   	      $str .= "name ILIKE '%";
   	      $str .= $_POST['state_keywords']."%'  AND  ";
  	  }
  	  if(!strlen($_POST["region_keywords"])==0){
  	      $str .= "region ILIKE '%";
 	       $str .= $_POST["region_keywords"]."%'  AND  ";
 	   }
   	 if(!strlen($_POST["div_keywords"])==0){
  	      $str .= "division ILIKE '%";
   	     $str .= $_POST["div_keywords"]."%'  AND  ";
 	   }
	 if(!strlen($_POST["min_popest2010"])==0){
 	       $str .= "popestimate2010 >= ";
 	       $str .= $_POST["min_popest2010"]."  AND  ";
	 }
	 if(!strlen($_POST["max_popest2010"])==0){
	   $str .= "popestimate2010 <= ";
     	   $str .= $_POST["max_popest2010"]."  AND  ";
   	 }
    	if(!strlen($_POST["min_popest2011"])==0){
    	    $str .= "popestimate2011 >= ";
  	     $str .= $_POST["min_popest2011"]."  AND  ";
    	}
    	if(!strlen($_POST["max_popest2011"])==0){
     	   $str .= "popestimate2011 <= ";
           $str .= $_POST["max_popest2011"]."  AND  ";
    	}
   	if(!strlen($_POST["min_huest2010"])==0){
    	    $str .= "huest_2010 >= ";
    	    $str .= $_POST["min_huest2010"]."  AND  ";
    	}
        if(!strlen($_POST["max_huest2010"])==0){
           $str .= "huest_2010 <= ";
           $str .= $_POST["max_huest2010"]."  AND  ";
        }
        if(!strlen($_POST["min_huest2011"])==0){
           $str .= "huest_2011 >= ";
           $str .= $_POST["min_huest2011"]."  AND  ";
        }
        if(!strlen($_POST["max_huest2011"])==0){
           $str .= "huest_2011 <= ";
           $str .= $_POST["max_huest2011"]."  AND  ";
        }
        $new_str = substr($str, 0, -6);
        $new_str .= " ;";
   }

   // if session is set, put them into variable
   if(isset($_SESSION["backup"])){
        $state_name = $_SESSION["backup"][0];
        $region = $_SESSION["backup"][1];
        $division = $_SESSION["backup"][2];
        $min_popest2010 = $_SESSION["backup"][3];
        $max_popest2010 = $_SESSION["backup"][4];
        $min_popest2011 = $_SESSION["backup"][5];
        $max_popest2011 = $_SESSION["backup"][6];
        $min_huest2010  = $_SESSION["backup"][7];
        $max_huest2010  = $_SESSION["backup"][8];
        $min_huest2011  = $_SESSION["backup"][9];
        $max_huest2011  = $_SESSION["backup"][10];
        session_destroy();
   }
   //read from query
   else if(pg_numrows($result) > 0)
   {
      $max_popest2010 = pg_result($result, 0, 0);
      $min_popest2010 = pg_result($result, 0, 1);
      $max_popest2011 = pg_result($result, 0, 2);
      $min_popest2011 = pg_result($result, 0, 3);
      $max_huest2010  = pg_result($result, 0, 4);
      $min_huest2010  = pg_result($result, 0, 5);
      $max_huest2011  = pg_result($result, 0, 6);
      $min_huest2011  = pg_result($result, 0, 7);

   }
?>
<b>SEARCH FOR NOTES</b><br /><br />
<table border=0 bgcolor='white' cellspacing=7 cellpadding=2>
<form name=input method='post' action='index.php'>
  <tr>
    <td>state name (keywords) </td   >
    <td><?php echo"<input type='text' name='state_keywords' style='width:200pxi' value='".$state_name."'/>";?></td>
  </tr>
  <tr>
    <td>region name (keywords) </td>
    <td><?php echo"<input type='text' name='region_keywords' style='width:200px;' value='".$region."'/>";?></td>
  </tr>
  <tr>
    <td>division name (keywords) </td>
    <td><?php echo"<input type='text' name='div_keywords' style='width:200px;' value='".$division."'/>";?></td>
  </tr>
  <tr>
    <td>pop estimate 2010  in range </td>
    <td><?php echo "<input type='text' name='min_popest2010' style='width:200px;' value='".$min_popest2010."' />"; ?></td>
    <td> , </td>
    <td><?php echo "<input type='text' name='max_popest2010' style='width:200px;' value='".$max_popest2010."' />"; ?></td>
  </tr>
  <tr>
    <td>pop estimate 2011 in range </td>
    <td><?php echo "<input type='text' name='min_popest2011' style='width:200px;' value='".$min_popest2011."' />"; ?></td>
    <td> , </td>
    <td><?php echo "<input type='text' name='max_popest2011' style='width:200px;' value='".$max_popest2011."' />"; ?></td>
  </tr>
  <tr>
    <td>housing estimate 2010 in range </td>
    <td><?php echo "<input type='text' name='min_huest2010' style='width:200px;' value='".$min_huest2010."' />"; ?></td>
    <td> , </td>
    <td><?php echo "<input type='text' name='max_huest2010' style='width:200px;' value='".$max_huest2010."' />"; ?></td>
  </tr>
  <tr>
    <td>housing estimate 2011 in range </td>
    <td><?php echo "<input type='text' name='min_huest2011' style='width:200px;' value='".$min_huest2011."' />"; ?></td>
    <td> , </td>
    <td><?php echo "<input type='text' name='max_huest2011' style='width:200px;' value='".$max_huest2011."' />"; ?></td>
  </tr>
  <tr>
    <td><input type='submit' name= 'SearchBtn' value='Search' /></td>
  </tr>
</form>
</table>
<br />
<?php
	//run query
        if(isset($_REQUEST["SearchBtn"])){
            $ret = pg_exec($db_handle, $new_str);
            if(!$ret){
          	 echo "<br />Query Failed!\n";
          	 var_dump($ret);
       	    }
       	    else{
		//print out table containing the query
		 $record_found = pg_numrows($ret);
		 echo "<p>$record_found records found!</p>";
           	 $q_array = pg_fetch_all_columns($ret,3);
          	  $region_array = pg_fetch_all_columns($ret, 1);
          	 $id_array = pg_fetch_all_columns($ret,0);
            	echo "<table border=1 bgcolor='white' cellspacing=7 cellpadding=2>";
            	echo "<tr><th>\"State\"</th><th>\"Region\"</th><th>\"Link\"</th></tr>";
            	for($i=0; $i <count($q_array); $i++){
            		if(strlen($q_array[$i])==0){
            	 	  $str_out = "UNKNOWN";
            		}
           		else{
               			$str_out = $q_array[$i];
           		}
            		echo "<tr><td>$str_out</td>";

            		if(strlen($region_array[$i])==0){
            	    	$str_out2 = "UNKNOWN";
            		}
            		else{
              	  	$str_out2 = $region_array[$i];
            		}
            		echo "<td>$str_out2</td>";
            		echo "<td><a href=\"inspect.php?state=".$id_array[$i]."\" method = 'post' action = 'inspect.php'>Inspect</a></td></tr>";
            	}
            	echo"</table>";

            }
       }

   
   pg_close($db_handle);
?>
</body>
</html>
