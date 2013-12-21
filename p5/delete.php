<html>
<body>
<?php
   include("header.php");
?>
<b>DROP ALL POPULATION AND HOUSING DATA</b><br />
<table border=0 bgcolor='white' cellspacing=7 cellpadding=2>
<form name=input method='post' action='delete.php'>
  <tr>
    <td>Remove all data? </td>
    <td>Yes<input type='radio' name='confirm' value='Yes' />
	No<input type='radio' name='confirm' value='No' checked /></td>
  </tr>
  <tr>
    <td><input type='submit' name="proceed" value='Proceed' /></td>
  </tr>
</form>
</table>
<br />
<?php
   // empty the POPULATION AND HOUSING table, print the number of items dropped
   $db_handle = pg_connect('dbname=cs564_patel host=postgres.cs.wisc.edu sslmode=require');
   if($_POST["confirm"] == 'Yes'){
	   $ret = pg_exec($db_handle, "TRUNCATE TABLE gooi.POP_HOUSING_ESTIMATE_STATE;");
	   if(!$ret){
	    echo "Truncate failed!";
	   }
	   else {
	      echo "All entries in the table is being deleted!";
	   }
   }
?>
</body>
</html>
