<?php 
session_start();

if (isset($_GET["error"])) {
echo("<pre>OAuth Error: " . $_GET["error"]."\n");
die;
}

//echo('<pre>set ID and secret</pre>');
// client id
$clientId = 'youreddit_api_clientId';
// client secret
$clientSecret = 'youreddit_api_clientSecret';
	
$authorizeUrl = 'https://ssl.reddit.com/api/v1/authorize';
$accessTokenUrl = 'https://ssl.reddit.com/api/v1/access_token';
    static $CLIENT_ID = 'youreddit_api_clientId';
    static $CLIENT_SECRET = 'youreddit_api_clientSecret';

$redirectUrl = "http://yourserver.com/reddit/";


require("Client.php");
require("GrantType/IGrantType.php");
require("GrantType/AuthorizationCode.php");


$client = new OAuth2\Client($clientId, $clientSecret, OAuth2\Client::AUTH_TYPE_AUTHORIZATION_BASIC);


if (isset($_GET["code"])) {
	echo('<pre>get code</pre>');
	$_SESSION['GOB']['code']=$_GET["code"];
	$_SESSION['GOB']['loggedin']=true;
}

if (!isset($_SESSION['GOB']['loggedin'])) {
	$_SESSION['GOB']['loggedin']=false;
	//echo "not loggged in";
}

if ($_SESSION['GOB']['loggedin']) {
	//echo "loggged in";
	$params = array("code" => $_SESSION['GOB']['code'], "redirect_uri" => $redirectUrl);

	$response = $client->getAccessToken($accessTokenUrl, "authorization_code", $params);

	$accessTokenResult = $response["result"];

	if (isset($_SESSION['GOB']['token'])) {
		$client->setAccessToken($_SESSION['GOB']['token']);
	} else {
		$client->setAccessToken($accessTokenResult["access_token"]);
		$_SESSION['GOB']['token'] = $accessTokenResult["access_token"];
	}

	$client->setAccessTokenType(OAuth2\Client::ACCESS_TOKEN_BEARER);

	$response = $client->fetch("https://oauth.reddit.com/api/v1/me.json");
	$_SESSION['GOB']['name'] = $response["result"]["name"];
	$_SESSION['GOB']['karma'] = $response["result"]["link_karma"];
}
?>