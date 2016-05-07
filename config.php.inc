<?php
error_reporting(E_ALL);
ini_set('display_errors', '1');

$mysql_db = "hotspot";
$mysql_host = "localhost"; //If the MySQL server is running on the same server as the web auth page
$mysql_user = "test"; //Change to the appropriate user that has access to DB/table
$mysql_pass = "654321"; //Change to the appropriate password for the user above
$table = "creds";  //Change to the appropriate table name

//UniFi server credentials
$unifiServer = "https://192.168.10.78:8443"; //Change to the IP/FQDN of your UniFi Server
//It's important to note that if this server is offsite, you need to have port 8443 forwarded through to it
$unifiUser = "your_unifi_username"; //Change to your UniFi Username
$unifiPass = "your_unifi_password"; //Change to your UniFi Password

//Establish connection the MySQL server using credentials as above.  If connection cannot be made, the process will fail and produce a troubleshooting error
$mysql = mysql_connect($mysql_host,$mysql_user,$mysql_pass);
	if(!$mysql) {
		die("Database connection failed: " . mysql_error());
	}

//Select the appropriate database for credential storage
$db_select = mysql_select_db($mysql_db,$mysql);
	if(!$db_select) {
		die("Database selection failed: " . mysql_error());
	}

//The function to authenticate a user using Username, Password, and Email as authentication paramaters then return a true boolena to the auth.php page
function auth_user($username,$password,$email,$table) {
	$query = "SELECT * FROM `$table` WHERE `username` = \"$username\"";
	$execute = mysql_query($query);
	while($row = mysql_fetch_assoc($execute)) {
		if(($row['password'] == $password) && ($row['email'] == $email)) {
			echo "true";
			return true;
		} else {
			echo "false";
			echo "Your username, password, or email was not found.<br />You will be redirected to the login page momentarily";
			sleep(5);
			header("Location: auth.php");
		}
	}
//If not requiring email as an authentication parameter, comment out the above while loop and uncomment the following while loop
/*
	while($row = mysql_fetch_assoc($execute)) {
		if($row['password'] == $password) {
			echo "true";
			return true;
		} else {
			echo "false";
			echo "Your username and/or password was not found.<br />You will be redirected to the login page momentarily";
			sleep(5);
			header("Location: auth.php");
		}
	}
*/
}



function sendAuthorization($id, $minutes, $url) {
	global $unifiServer;
	global $unifiUser;
	global $unifiPass;

	// Start Curl for login
	$ch = curl_init();
	// We are posting data
	curl_setopt($ch, CURLOPT_POST, TRUE);
	// Set up cookies
	$cookie_file = "/tmp/unifi_cookie";
	curl_setopt($ch, CURLOPT_COOKIEJAR, $cookie_file);
	curl_setopt($ch, CURLOPT_COOKIEFILE, $cookie_file);
	// Allow Self Signed Certs
	curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
	curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, FALSE);
	// Force SSL3 only
	curl_setopt($ch, CURLOPT_SSLVERSION, 3);
	// Login to the UniFi controller
	curl_setopt($ch, CURLOPT_URL, "$unifiServer/login");
	curl_setopt($ch, CURLOPT_POSTFIELDS,"login=login&username=$unifiUser&password=$unifiPass");
	curl_exec ($ch);
	curl_close ($ch);

	// Send user to authorize and the time allowed
	$data = json_encode(array(
		'cmd'=>'authorize-guest',
        'mac'=>$id,
        'minutes'=>$minutes));
	$ch = curl_init();
	// We are posting data
	curl_setopt($ch, CURLOPT_POST, TRUE);
	// Set up cookies
	$cookie_file = "/tmp/unifi_cookie";
	curl_setopt($ch, CURLOPT_COOKIEJAR, $cookie_file);
	curl_setopt($ch, CURLOPT_COOKIEFILE, $cookie_file);
	// Allow Self Signed Certs
	curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
	curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, FALSE);
	// Force SSL3 only
	curl_setopt($ch, CURLOPT_SSLVERSION, 3);
	// Make the API Call
	curl_setopt($ch, CURLOPT_URL, $unifiServer.'/api/cmd/stamgr');
	curl_setopt($ch, CURLOPT_POSTFIELDS, 'json='.$data);
	curl_exec ($ch);
	curl_close ($ch);
  
	// Logout of the connection
	$ch = curl_init();
	// We are posting data
	curl_setopt($ch, CURLOPT_POST, TRUE);
	// Set up cookies
	$cookie_file = "/tmp/unifi_cookie";
	curl_setopt($ch, CURLOPT_COOKIEJAR, $cookie_file);
	curl_setopt($ch, CURLOPT_COOKIEFILE, $cookie_file);
	// Allow Self Signed Certs
	curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
	curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, FALSE);
	// Force SSL3 only
	curl_setopt($ch, CURLOPT_SSLVERSION, 3);
	// Make the API Call
	curl_setopt($ch, CURLOPT_URL, $unifiServer.'/logout');
	curl_exec ($ch);
	curl_close ($ch);
	echo "Login successful, I should redirect to: " . $url; //$_SESSION['url'];
	sleep(8); // Small sleep to allow controller time to authorize
	header('Location: ' . $url);//$_SESSION['url']);
}
?>