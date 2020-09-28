#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "hw_timer.h"          
const byte zcPin = 12;
const byte pwmPin = 13;  

byte fade = 1;
byte state = 1;
byte tarBrightness = 255;
byte curBrightness = 0;
byte zcState = 0; // 0 = ready; 1 = processing;


/* Put your SSID & Password */
const char* ssid = "NodeMCU_Server";  // Enter SSID here
const char* password = "esp8266";  //Enter Password here

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);



void setup() {
  Serial.begin(115200);
  
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  
  server.on("/", handle_OnConnect);
  server.on("/zero", zerop);
  server.on("/twentyfive", twentyfivep);
  server.on("/fifty", fiftyp);
  server.on("/seventyfive", seventyfivep);
  server.on("/hundred", hundredp);
  
  server.onNotFound(handle_NotFound);
  
  server.begin();
  Serial.println("HTTP server started");
  
  pinMode(zcPin, INPUT_PULLUP);
  pinMode(pwmPin, OUTPUT);
  attachInterrupt(zcPin, zcDetectISR, RISING);    // Attach an Interupt to Pin 2 (interupt 0) for Zero Cross Detection
  hw_timer_init(NMI_SOURCE, 0);
  hw_timer_set_func(dimTimerISR);

}
void loop() {
  server.handleClient();
}
void handle_OnConnect() {
  tarBrightness = 0;
    server.send(200, "text/html", SendHTML(tarBrightness));
  
}
void zerop() {
  tarBrightness = 0;
  
  server.send(200, "text/html", SendHTML(tarBrightness)); 
}

void twentyfivep() {
  tarBrightness = 25;
  
  server.send(200, "text/html", SendHTML(tarBrightness)); 
}
void fiftyp() {
  tarBrightness = 50;
  
  server.send(200, "text/html", SendHTML(tarBrightness)); 
}
void seventyfivep() {
  tarBrightness = 75;
  
  server.send(200, "text/html", SendHTML(tarBrightness)); 
}
void hundredp() {
  tarBrightness = 100;
  
  server.send(200, "text/html", SendHTML(tarBrightness)); 
}
void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
  
}

void dimTimerISR() {
    if (fade == 1) {
      if (curBrightness > tarBrightness || (state == 0 && curBrightness > 0)) {
        --curBrightness;
      }
      else if (curBrightness < tarBrightness && state == 1 && curBrightness < 255) {
        ++curBrightness;
      }
    }
    else {
      if (state == 1) {
        curBrightness = tarBrightness;
      }
      else {
        curBrightness = 0;
      }
    }
    
    if (curBrightness == 0) {
      state = 0;
      digitalWrite(pwmPin, 0);
    }
    else if (curBrightness == 255) {
      state = 1;
      digitalWrite(pwmPin, 1);
    }
    else {
      digitalWrite(pwmPin, 1);
      
    }
    
    zcState = 0;
}

void zcDetectISR() {
  if (zcState == 0) {
    zcState = 1;
  
    if (curBrightness < 255 && curBrightness > 0) {
      digitalWrite(pwmPin, 0);
      
      int dimDelay = 30 * (255 - curBrightness) + 400;//400
      hw_timer_arm(dimDelay);
    }
  }
}


String SendHTML(uint8_t dimstat){
 String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #1abc9c;}\n";
  ptr +=".button-on:active {background-color: #16a085;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP8266 Web Server</h1>\n";

    //>>>>>>working on the HTML part
    
   if(dimstat)
  {ptr +="<p>LED1 Status: ON</p><a class=\"button button-off\" href=\"/zero\">0%</a>\n";}
  else{
    
  }
    if(dimstat)
  {ptr +="<p>LED1 Status: ON</p><a class=\"button button-off\" href=\"/twentyfive\">25%</a>\n";}
   if(dimstat)
  {ptr +="<p>LED1 Status: ON</p><a class=\"button button-off\" href=\"/fifty\">50%</a>\n";}
    if(dimstat)
  {ptr +="<p>LED1 Status: ON</p><a class=\"button button-off\" href=\"/seventyfive\">75%</a>\n";}
    if(dimstat)
  {ptr +="<p>LED1 Status: ON</p><a class=\"button button-off\" href=\"/hundred\">100%</a>\n";}
  
 

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
