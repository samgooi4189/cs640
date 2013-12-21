<html>
<head>
<meta http-equiv="content-type" content="text/html; charset=ISO-8859-1">

<title>CS 564 PHP Project: Detail Search Result</title>
</head>
<body>

Here are the detail search results for

<?php
	function getTypeNow($str, $stringFlag){
		$itsNULL = 0;
		if(!is_string($str) || empty($str)){
			$str = "NULL";
			$itsNULL = 1;
		}
		if($stringFlag == 1 && $itsNULL == 0)
			return "' ".$str." '";
		else
			return ($str);
	}
	
    if(!session_id()) session_start();
    //connect to the database
    $dbconn = pg_connect("host=postgres.cs.wisc.edu port=5432 dbname=cs564_patel");
    if ($dbconn == FALSE) {
        print "Connection to database failed.<br/>";
    }

    $id = -1;
    //get the id of the tuple to find details for (via http GET)
    if(!isset($_REQUEST["EditBtn"]) && !isset($_REQUEST["CommitBtn"]) && !isset($_REQUEST["SubmitBtn"]) && !isset($_REQUEST["CancelBtn"])){
      $id = $_GET["state"];
      $_SESSION["state"]= $id;
    }
    else{
      $id = $_SESSION["state"];
    }
    echo "state_id ".$id."<br/><br/>"; 

    // generate update query
    if(isset($_REQUEST["CommitBtn"])){
       $update_q = "UPDATE gooi.pop_housing_estimate_state SET state = ";
       $update_q .= getTypeNow($_SESSION["updates"][0], 0);
         $update_q .= ", region = ";
       $update_q .= getTypeNow($_SESSION["updates"][1], 1);
       $update_q .= ", division = ";
       $update_q .= getTypeNow($_SESSION["updates"][2], 1);
       $update_q .= ", name = ";
       $update_q .= getTypeNow($_SESSION["updates"][3], 1);
       $update_q .= ", popestimate2010 = ";
       $update_q .= getTypeNow($_SESSION["updates"][4], 0);
       $update_q .= ", popestimate2011 = ";
       $update_q .= getTypeNow($_SESSION["updates"][5], 0);
       $update_q .= ", huest_2010 = ";
       $update_q .= getTypeNow($_SESSION["updates"][6], 0);
       $update_q .= ", huest_2011 = ";
       $update_q .= getTypeNow($_SESSION["updates"][7], 0);
       $update_q .= " WHERE state = ";
      $update_q .= $id;
       $update_q .= ";";
       $ret_update = pg_exec($dbconn, $update_q);
       
    }
    //construct the query for lookup
    $query = "SELECT * FROM gooi.pop_housing_estimate_state where state = ".$id.";";

    //query the database
    //fetch the row in the table that was returned by the query and display the results
    $ret = pg_exec($dbconn, $query);

    if(!$ret){
       echo "<p>Querry Failed!</p>";
    }

    $record_arry = pg_fetch_row($ret, 0);

    //get all the inputs into session before commit
    if(isset($_REQUEST["SubmitBtn"]) ){
        $_SESSION["updates"] = array($id, $_GET["region"], $_GET["division"], $_GET["stname"], $_GET["popest2010"], $_GET["popest2011"], $_GET["huest2010"], $_GET["huest2011"]);
	echo "<br/><p>ARE YOU SURE TO MAKE THE CHANGES BELOW? </p><br/>";
    }
    // drop all the inputs
    if(isset($_REQUEST["CancelBtn"])){
        //reset the query
        $_SESSION["updates"]= array();

    }
    
    //load into form from session
    if(COUNT($_SESSION["updates"])>0 && isset($_REQUEST["SubmitBtn"])){
      $state = $_SESSION["updates"][0];
       $region = $_SESSION["updates"][1];
      $division = $_SESSION["updates"][2];
      $stname = $_SESSION["updates"][3];
      $popest2010 = $_SESSION["updates"][4];
      $popest2011 = $_SESSION["updates"][5];
      $huest2010 = $_SESSION["updates"][6];
      $huest2011 = $_SESSION["updates"][7];
    }
    else if(COUNT($record_arry) > 0 ){
      $state = $record_arry[0];
       $region = $record_arry[1];
      $division = $record_arry[2];
       $stname = $record_arry[3];
       $popest2010 =$record_arry[4];
       $popest2011 = $record_arry[5];
       $huest2010 = $record_arry[6];
       $huest2011 = $record_arry[7];
    }

?>

<table border=0 bgcolor='white' cellspacing=7 cellpadding=2>
<form name=input method='get' action='inspect.php'>
  <tr>
    <td>State ID : </td>
    <td><?php
		if(strlen($state)==0)
			echo "UNKNOWN";
        	else    
			echo $state;
        ?>
    </td>
  </tr>
  <tr>
    <td>Region Name : </td>
    <td><?php
    if(isset($_REQUEST["EditBtn"])){
        echo "<input type='text' name='region' style='width:200px;' value='".$region."' />";
    }
    else{
	if(strlen($region)==0)
		echo "UNKNOWN";
        else
       		echo $region;
    }
    ?>
    </td>
  </tr>
  <tr>
    <td>Division Name : </td>
    <td><?php
    if(isset($_REQUEST["EditBtn"])){
        echo "<input type='text' name='division' style='width:200px;' value='".$division."' />";
    }
    else{
	if(strlen($division)==0)
		echo "UNKNOWN";
        else
        	echo $division;
    }
    ?>
    </td>
  </tr>
  <tr>
    <td>State Name : </td   >
    <td><?php
        if(isset($_REQUEST["EditBtn"])){
        echo"<input type='text' name='stname' style='width:200px;' value='".$stname."'/>";
        }
        else{
		if(strlen($stname)==0)
			echo "UNKNOWN";
        	else
            		echo $stname;
        }
        ?>
    </td>
  </tr>

  <tr>
    <td>Pop Estimate 2010 : </td>
    <td><?php
    if(isset($_REQUEST["EditBtn"])){
        echo "<input type='text' name='popest2010' style='width:200px;' value='".$popest2010."' />";
    }
    else{
	if(strlen($popest2010)==0)
		echo "UNKNOWN";
        else
        	echo $popest2010;
    }
    ?>
    </td>
  </tr>
  <tr>
    <td>Pop Estimate 2011 : </td>
    <td><?php
    if(isset($_REQUEST["EditBtn"])){
        echo "<input type='text' name='popest2011' style='width:200px;' value='".$popest2011."' />";
    }
    else{
	if(strlen($popest2011)==0)
		echo "UNKNOWN";
        else
        	echo $popest2011;
    }
    ?>
    </td>
  </tr>
  <tr>
    <td>Housing Estimate 2010 : </td>
    <td><?php
    if(isset($_REQUEST["EditBtn"])){
            echo "<input type='text' name='huest2010' style='width:200px;' value='".$huest2010."' />";
    }
    else{
	if(strlen($huest2010)==0)
		echo "UNKNOWN";
        else
        	echo $huest2010;
    }
    ?>
    </td>
  </tr>
  <tr>
    <td>Housing Estimate 2011 : </td>
    <td><?php
    if(isset($_REQUEST["EditBtn"])){
        echo "<input type='text' name='huest2011' style='width:200px;' value='".$huest2011."' />";
    }
    else{
	if(strlen($huest2011)==0)
		echo "UNKNOWN";
        else
        	echo $huest2011;
    }
    ?>
    </td>
  </tr>
  <tr>
  <td><?php
    if(isset($_REQUEST["EditBtn"])){
        echo "<input type='submit' name= 'SubmitBtn' value='Submit' />";
    }
    else if(isset($_REQUEST["SubmitBtn"])){
        echo "<input type='submit' name= 'CommitBtn' value='Commit' />";
    }
    else{
        echo "<input type='submit' name= 'EditBtn' value='Edit' />";
     }
    ?>
    </td><td>
        <input type='submit' name = 'CancelBtn' value='Cancel'/>
    </td><td>
        <input type="button" name="BackBtn" Value="Go Back to Index" onclick="window.location.href='index.php'"/>
    </td>
  </tr>
</form>
</table>
<br />
<?php
	//print status of update
	if(isset($_REQUEST["CommitBtn"])){
		if($ret_update){
			echo "<p>UPDATE SUCCESS!</p>";
		}
		if(!$ret_update){
           		echo "<p>Update FAIL!!</p>";
       		}
	}

	

?>
</tbody>
</table>
<!--provide a link to the previous page-->
</body>
</html>
