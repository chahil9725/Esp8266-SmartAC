/**************************************
  This Code is Written By Chahil Patel
  and Allowed To Modifiy .
  Contect:chahil9725@gmail.com         *
***************************************/


#include <FS.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include<FirebaseArduino.h>
#include <EEPROM.h>
#include <WiFiClient.h>
#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Kelvinator.h>

//const char* essid = "";    //SSID  to Connect WiFi
//const char* epassword = ""; //Password TO Connect WiFi

const char* APssid = "AC REMOTE";   //Config Portal SSID
const char* APpassword = "12345678"; //Config Portal Password

//String token = "";    //User Token Comes From WiFi
String firebaseurl = ""; //Defiend in Code Hear
String firebasetoken = ""; // Defiend in Code Hear
int irled = 4;          // IR LED PIN
int button = 0;         // Config portal Pin

String st;         // Web Server Variable
String content;   // Web Server Variable
int statusCode;   // Web Server Variable

ESP8266WebServer server(80); //Define Webserver

bool eepromandwifi();      //Read from EEPROM  End Return True/False and start wifi
bool testwifi();    // Test WiFi End Return True/False
void startAP();     //Start SoftAP And Launch Config() As Config Portal
void Config();      //Take Value From Web Server And Store In EEPROM

void led(int,int);  //Led Blynker led(numberoftime,delay);


void setup() {
  Serial.begin(115200);
  pinMode(button, INPUT_PULLUP);
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("...........................................");
  Serial.println("                   Start                   ");
  if (eepromandwifi() == true) // 
  {
   
    if (testwifi() == false)
    {
      Serial.println("Lunching Config Portal");
      led(5,100);
      startAP();

    }
  }
  if (eepromandwifi() == false)
  {
    led(5,100);
    startAP();
    
  }
}

void loop() 
{
  server.handleClient();
  if (digitalRead(button) == LOW)
  {
    delay(2000);
    led(5,100);
    startAP();
  }

}
void led(int a,int d)
{
  for (int i = 0;i > a; i++)
    {
      digitalWrite(LED_BUILTIN, LOW);
      delay(d);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(d);
    }
}

bool testwifi()
{
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.print("Local IP: ");
      Serial.println(WiFi.localIP());
      return true;
    }
    delay(400);
    Serial.print(".");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;

}
bool eepromandwifi()
{
  EEPROM.begin(512);
  delay(10);
  Serial.println("Reading EEPROM ssid");
  String epassword = "";
  String essid;
  String token;
  for (int i = 0; i < 32; ++i)
  {
    essid += char(EEPROM.read(i));
  }
  Serial.println("SSID: ");
  Serial.println(essid);

  Serial.print("Reading EEPROM Password");
  for (int i = 32; i < 96; ++i)
  {
    epassword += char(EEPROM.read(i));
  }
  Serial.println("PASS: ");
  Serial.println(epassword);
  Serial.print("Reading EEPROM token");
  for (int i = 96; i < 160; ++i)
  {
    token += char(EEPROM.read(i));
  }
  Serial.println("Token: ");
  Serial.println(token);
  if ( essid.length() > 1 && epassword.length() > 1 && token.length() > 1 )
  {
     WiFi.begin(essid.c_str(), epassword.c_str());
    return true;
  }
  if ( essid.length() < 1 && epassword.length() < 1 && token.length() < 1 )
  {
    return false;
  }
}

void startAP()
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  IPAddress ip(10, 1, 1, 1);
  IPAddress gateway(10, 1, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(APssid, APpassword);
  Serial.println("................SoftAP Mode...................");
   Serial.println("  ");
  Serial.println("SSID: ");
  Serial.println(APssid);
  Serial.print("  ");
  Serial.println("Password: ");
  Serial.println(APpassword);
  Serial.print("  ");
  Serial.println("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("  ");
  server.begin();
  Serial.println("Server started");
  Config();

}

void Config()
{
  server.on("/", []() {
   
    server.send(200, "text/html", "Hello From Chahil");
  });
  server.on("/setting", []() {
    String qssid = server.arg("ssid");
    String qpass = server.arg("pass");
    String qtoken = server.arg("token");

    if (qssid.length() > 0 && qpass.length() > 0 && qtoken.length() > 0)
    {
      Serial.println("clearing eeprom");
      for (int i = 0; i < 160; ++i)
      {
        EEPROM.write(i, 0);
      }

      Serial.println(qssid);
      Serial.println("");
      Serial.println(qpass);
      Serial.println("");
      Serial.println(qtoken);
      Serial.println("");

      Serial.println("writing eeprom ssid:");
      for (int i = 0; i < qssid.length(); ++i)
      {
        EEPROM.write(i, qssid[i]);
        Serial.print("Wrote: ");
        Serial.println(qssid[i]);
      }
      Serial.println("writing eeprom pass:");
      for (int i = 0; i < qpass.length(); ++i)
      {
        EEPROM.write(32 + i, qpass[i]);
        Serial.print("Wrote: ");
        Serial.println(qpass[i]);
      }
      Serial.println("writing eeprom token:");
      for (int i = 0; i < qtoken.length(); ++i)
      {
        EEPROM.write(96 + i, qtoken[i]);
        Serial.print("Wrote: ");
        Serial.println(qtoken[i]);
      }

      EEPROM.commit();
      content = "{Done}";
      statusCode = 200;
    }
    else {
      content = "{Error}";
      statusCode = 404;
      Serial.println("Error");
    }

    server.send(statusCode, "application/json", content);
  });

  server.on("/cleareeprom", []() {
    content = "<!DOCTYPE HTML>\r\n<html>";
    content += "<p>Clearing the EEPROM</p></html>";
    server.send(200, "text/html", content);
    Serial.println("clearing eeprom");
    for (int i = 0; i < 96; ++i) {
      EEPROM.write(i, 0);
    }
    EEPROM.commit();
  });

   server.on("/restart", []() {
    led(3,200);
    server.send(200, "text/plain", "Restart in 5 Second");
    delay(5000);
    ESP.reset();
    
  });
}



