// Icon images are stored in tabs
// more than one icon can be in a header file

// Arrays containing FLASH images can be created with UTFT library tool:
// (libraries\UTFT\Tools\ImageConverter565.exe)
// Convert to .c format then copy into a new tab

/*
 This sketch demonstrates loading images from arrays stored in program (FLASH) memory.

 Works with TFT_eSPI library here:
 https://github.com/Bodmer/TFT_eSPI

 This sketch does not use/need any fonts at all...

 Code derived from ILI9341_due library example

 Make sure all the display driver and pin comnenctions are correct by
 editting the User_Setup.h file in the TFT_eSPI library folder.

 #########################################################################
 ###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
 #########################################################################
*/

#include <Wire.h>                  // installed by default
#include <ESP8266WiFi.h>           // version: 2.3.0
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

//start display specific
#include <TFT_eSPI.h>       // Hardware-specific library
TFT_eSPI tft = TFT_eSPI();  // Invoke custom library
// Include the header files that contain the icons
#include "newmail.h"
#include "nomail.h"
#include "reddit_guy.h"
#include "envelope_grey.h"
#include "envelope_red.h"
#include "up_arrow_black.h"
#include "up_arrow_green.h"


//end display specific

//------- Replace the following! -------
char ssid[] = "xxxxxx"; // your network SSID (name)
char password[] = "xxxxxx";   // your network password

String userID = "u/yourusername"; //you might wanna change this!
String url="http://yourdomain.com/reddit_data.php"; //page to scrape, change this too!
//------- ----------------------- ------


unsigned long api_mtbs = 60000; //mean time between api requests (60 seconds)
unsigned long api_lasttime;     //last time api request has been done

#define INTERVAL 60
#define TFT_GREY 0xA9A9A9 //darkgrey color
unsigned long last_millis = 0;
int karma_total;
int karma_prev;
bool new_mail;
int reddit_mood=1;
int reddit_mood_prev=1;
String message_part;
String mail_status;
int inbox_count=0;
unsigned long count=0;

const int buzzer = D8; //peizo buzzer

void setup() {
  Serial.begin(115200);
  
  pinMode(buzzer, OUTPUT); //for new data- beep!
  
  tft.begin();
  tft.setRotation(4);	// landscape
  // Swap the colour byte order when rendering
  tft.setSwapBytes(true);
  //clear screen
  tft.fillScreen(TFT_BLACK);
  // Draw the icons
  tft.pushImage(12, 16, reddit_guyWidth, reddit_guyHeight, reddit_guy);
  tft.setCursor(0,12); //bottom of text for fonts
  tft.setTextColor(TFT_WHITE);
  tft.println("ESP8266");
  Serial.println("done");
  
  delay(2000);
  // Set WiFi to station mode and disconnect from an AP if it was previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  tft.print("Connecting Wifi...");
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("Connected to WiFi");
  Serial.println("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
  Serial.print("tft width = ");
  Serial.println(tft.width());
  Serial.print("tft height = ");
  Serial.println(tft.height());
  tft.fillScreen(TFT_BLACK);
  // Draw some backgrounds
  tft.pushImage(12, 16, reddit_guyWidth, reddit_guyHeight, reddit_guy);
  delay(2000);
  tft.pushImage(0, 0, newmailWidth, newmailHeight, newmail);
  //test buzzer
  tone(buzzer, 4000);
  delay(50); 
  noTone(buzzer);
  //load initial data
  gethttp_data();
  karma_prev=karma_total; //initialize karma_prev
  //load initial graphics
  int loops=7;
  for(int i=0; i<loops;i++) {
     upVoteAnimation();
     delay(random(0,100));
  }
  displayMail();  //background screen gets redrawn here
  displayKarma();
  displayUser();
  //end setup 
}

void loop() {
  // Loop filling and clearing screen
  count=(api_lasttime + api_mtbs)-millis(); //countdown timer to refresh
  if(count>0 && count <60001) { count = (count/1000)+1; } else { count=0; }
  displayCount();
  if (millis() > api_lasttime + api_mtbs)  {
    api_lasttime = millis();
    gethttp_data();
    if(karma_total>karma_prev) {
      for(int i=0; i<karma_total-karma_prev;i++) {
        upVoteAnimation();
        delay(random(200,500));
      }
    }
    displayMail();  //background screen gets redrawn here
    displayKarma();
    displayUser();
    playBuzzer();
  }
  delay(1000); //loop delay
}

void upVoteAnimation() {
  int i;
  float j=1.0;
  int startY = tft.height()+up_arrow_greenHeight; //enough to get off screen
  tft.height();
  //start at random x
  int startX = random(0, (tft.width()-up_arrow_blackWidth));
  //draw arrows
  for(i=startY; i > 0-up_arrow_greenHeight; i = i-(up_arrow_blackHeight/2)) {
    tft.pushImage(startX, i, up_arrow_greenWidth, up_arrow_greenHeight, up_arrow_green);
    tone(buzzer, 800+j); //tone rises with delay j
    delay(j);
    noTone(buzzer);
    j=j*1.6;
    tft.pushImage(startX, i, up_arrow_greenWidth, up_arrow_greenHeight, up_arrow_black);
//    Serial.print("loop j: ");
//    Serial.println(j);
  }
  tone(buzzer, 8000);
  delay(500);
  noTone(buzzer);
}
  
void displayCount() {
  int16_t x = tft.width();
  int16_t y = 10; //font height is 8px
  tft.fillRect(0, 0 , tft.width()-36, y, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(1,1);
  tft.setTextColor(TFT_GREY);
  tft.print(count);
}

void displayKarma() {
//  tft.fillRect(0, 12, tft.width(), 24, TFT_BLACK);
  tft.setTextSize(2); //height = 16px
  tft.setCursor(6,96); //shadow
  tft.setTextColor(TFT_WHITE); 
  tft.print("karma:");
  tft.println(karma_total);
  tft.setCursor(4,96); //shadow
  tft.setTextColor(TFT_GREEN); 
  tft.print("karma:");
  tft.println(karma_total);
  tft.setCursor(5,95); //main text
  tft.setTextColor(TFT_BLUE);
  tft.print("karma:");
  tft.println(karma_total);
}

void displayMail() {
  tft.setTextSize(2);
  tft.setCursor(tft.width()-36+10,7);
  if(new_mail>0) {
    tft.setTextColor(TFT_BLACK); //we have mail!
    //update background image
    tft.pushImage(0, 0, newmailWidth, newmailHeight, newmail);
    //update icon
    tft.pushImage(90, 3, envelope_redWidth,envelope_redHeight, envelope_red);
    //update text (number of mail messages)
    tft.println(inbox_count);
  }
  else {
    //no new mail
    tft.setTextColor(TFT_GREY);
    //update background image
    tft.pushImage(0, 0, nomailWidth, nomailHeight, nomail);
    //update icon
    tft.pushImage(90, 3, envelope_greyWidth,envelope_greyHeight, envelope_grey);
    //update text (number of mail messages)
    tft.println(inbox_count);
  }
}

void displayUser() {
  int16_t x = tft.width();
  int16_t y = tft.height();
  tft.fillRect(0, 116 , x, y, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(19,116);
  tft.setTextColor(TFT_WHITE);
  tft.print(userID);
}

void playBuzzer() {
    if(karma_total>karma_prev) {
      //play buzzer
      int i;
      int diff = karma_total-karma_prev; //number of upvotes
      for(i=0;i<diff;i++) { 
        tone(buzzer, 4000);
        delay(50); 
        noTone(buzzer);     // Stop sound...
      }
      karma_prev=karma_total; //reset prev karma with current karma
    }
   else {
  }
}

void gethttp_data() {
    //open a web page and read the data
    HTTPClient http;
    String url="http://plurimediagroup.com/reddit_data.php";
    http.begin(url);
    http.addHeader("Content-Type","text/plain");
    int httpCode=http.GET();
    String data=http.getString();
    String lastmessage=data;
    http.end();
    
    //Serial.print("last message: ");
    //Serial.println(lastmessage);
    String search_string = "total_karma:";
    // find the index position of the start of the substring
    int search_string_pos = lastmessage.indexOf(search_string);
    if (lastmessage.substring(search_string_pos,search_string_pos+12) == "total_karma:") {
      //clip off string
      message_part = lastmessage.substring(search_string_pos+12);
      //find karma data terminator
      search_string = "<";
      search_string_pos = message_part.indexOf(search_string);
      // set the final data variable to display
      //convert to integer
      karma_total = (message_part.substring(0,search_string_pos)).toInt();
      
      //see if new mail
      search_string = "new_mail:";      
      search_string_pos = message_part.indexOf(search_string);
      message_part = message_part.substring(search_string_pos+9);
      search_string = "<";
      search_string_pos = message_part.indexOf(search_string);
      mail_status = message_part.substring(0,search_string_pos);
      if(mail_status=="FALSE") {
        new_mail = 0;
        inbox_count = 0; //reset mail counter
      }
      else if(mail_status=="TRUE") {
       new_mail=1;
      }
      else {
       new_mail=-1; 
      }
      if(new_mail) {
        //get new_mail count
        search_string = "inbox_count:";      
        search_string_pos = message_part.indexOf(search_string);
        message_part = message_part.substring(search_string_pos+12);
        search_string = "<";
        search_string_pos = message_part.indexOf(search_string);
        inbox_count = (message_part.substring(0,search_string_pos)).toInt();
      }
      Serial.print("karma: ");
      Serial.println(karma_total);
      Serial.print("Mail:");
      Serial.println(inbox_count);
  }
 }
 
