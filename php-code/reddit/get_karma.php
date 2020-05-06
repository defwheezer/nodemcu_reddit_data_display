<?php
session_start();
$_SESSION['user_id'] = "ivoidwarranty"; //change ivoidwarranty to your whatever
$_COOKIE['user_id'] = "ivoidwarranty"; //change ivoidwarranty to your whatever
// echo "<pre>_SESSION:";
// var_dump($_SESSION);
// echo "</pre>";
// echo "<pre>_COOKIE:";
// var_dump($_COOKIE);
// echo "</pre>";

if(!isset($_SESSION['user_id']) && isset($_COOKIE['user_id'])) {
    //echo "!isset(_SESSION['user_id']) && isset(_COOKIE['user_id']<br/>";
	$_SESSION['user_id'] = $_COOKIE['user_id'];
}
//isset($_SESSION['user_id']);

require_once("reddit_auth.php");
require_once("reddit.php");
//echo "Start Reddit php...<br/>";
$response_arr = get_auth();
$access_token = $response_arr["access_token"];
$token_type = $response_arr["token_type"];
$expires_in = $response_arr["expires_in"];
$scope = $response_arr["scope"];

//echo "access_token: " . $access_token . "</br>";
$reddit = new reddit();
$userData = $reddit->getUser();
$total_karma = ($userData->link_karma)+($userData->comment_karma);

//this is what prints out to the page "reddit.php"
echo "<pre>";
echo "total_karma:$total_karma";
echo "</pre>";

echo "<pre>";
echo "new_mail:";
if($userData->has_mail) {
	echo "TRUE";
}
else {
	echo "FALSE";
}
$inbox_count = $userData->inbox_count;
$inbox_count;
echo "</pre>";

echo "<pre>";
echo "inbox_count:$inbox_count";
echo "</pre>";

echo "<pre>";
echo var_dump($userData);
echo "</pre>";
?>