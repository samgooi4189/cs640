<html>
<body>
<?php
   include("header.php");
?>
<b>LOAD US POPULATION AND HOUSING DATA</b><br />
<table border=0 bgcolor='white' cellspacing=7 cellpadding=2>
<form name=input method='post' action='load.php'>
  <tr>
    <td>Path to CSV File</td>
    <td><input type='text' name='path' style='width:400px;' /></td>
  </tr>
  <tr>
    <td><input type='submit' name="LoadBtn" value='Load' /></td>
  </tr>
</form>
</table>
<br />
<?php
   // load data at path given into table
   // display number of records loaded and the new total number of records
   if(isset($_REQUEST["LoadBtn"])){
       if(strlen($_POST["path"])>0){
           $path = "./".$_POST["path"];
       }
   }
   $db_handle = pg_connect('dbname=cs564_patel host=postgres.cs.wisc.edu sslmode=require');
   $row = 1;
   $is_success = 0;
   $ret = pg_query($db_handle, "BEGIN TRANSACTION;");
   if(!$ret){
    echo "Begin transaction fail\n";
   }

   //insert record one by one
   if (($handle = fopen($path, "r")) !== FALSE  ) {
           while (($data = fgetcsv($handle, 1000, ",")) !== FALSE) {
                if($row == 1){
                    $row++;
                    continue;
                }
                 $str = "INSERT INTO gooi.POP_HOUSING_ESTIMATE_STATE ";
                 $strv = "VALUES (";


                $num = count($data);
                $row++;
                for ($c=0; $c < $num; $c++) {
                    if($c >=1 && $c <=3){
                        $data[$c] = "'$data[$c]'";
                    }
                    if(strlen($data[$c]) == 0){
                        $data[$c] = "NULL";
                    }
                    $strv .= "$data[$c],";
                }
                $strv[strlen($strv)-1] = ')';

                $str .= $strv;
                $str .= ';';


                $ret = pg_exec($db_handle, $str);
                 if(!$ret){
			$is_success = 1;
                 }
		else{
			$is_success = 0;
		}
		
           }
	   
           // print status of overall insert
	   $row = $row - 2;
	   if($is_success == 0){
		$ret = pg_exec($db_handle, "COMMIT;");
            	if(!$ret){
                	echo "<p>Commit fail</p>";
            	}
		else
	        	echo "<p>$row records inserted!</p>";
	   }		
	   else{
		$ret = pg_exec($db_handle, "ROLLBACK;");
            	if(!$ret){
			echo "<p>Rollback fail</p>";		
		}
		echo "<p>The loading had failed! / The file already exist!</p>";
	   }
            
               fclose($handle);
     }
    else{
	if(isset($_REQUEST["LoadBtn"]))
     		echo "Path is Invalid!";
     }



?>
</body>
</html>
