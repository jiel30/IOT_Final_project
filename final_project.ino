#include <SparkFun_VEML6075_Arduino_Library.h>
#include "WiFiEsp.h"
#include <SparkFun_RHT03.h>
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(10, 11); // RX, TX 
#endif

// value setting for uv light sensor
VEML6075 uv; // Create a VEML6075 object
float light = 0;
int light_time = 0;
bool low_light = false;
bool high_light = false;

// value setting for soil moisture sensor
float value = 0; //value for storing moisture value 
float percent = 0;
int soilPin = A0;//Declare a variable for the soil moisture sensor 
int soilPower = 7;//Variable for Soil moisture Power
bool low_water = false;

// value setting for WIFI module
const int RHT03_DATA_PIN = 4; 
RHT03 rht;
char ssid[] = "XP&JW"; 
char pass[] = "family01";
int status = WL_IDLE_STATUS; 
char server[] = "54.219.65.149";
char var[200] = "TEMP";
char get_request[200]; 
WiFiEspClient client;


void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  rht.begin(RHT03_DATA_PIN);
  // setting up for soil moisture
  pinMode(soilPower, OUTPUT);
  digitalWrite(soilPower, LOW);

  // setting up for uv light
  Wire.begin();
  if (uv.begin() == false){
    Serial.println("Unable to communicate with VEML6075.");
    while(1);
  }

  // setting up for WIFI module
  WiFi.init(&Serial1);
  if (WiFi.status() == WL_NO_SHIELD) {
  Serial.println("WiFi shield not present"); // don't continue
  while (true);
  }
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: "); 
    Serial.println(ssid);
// Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
    }
  Serial.println("You're connected to the network"); 
//  printWifiStatus();
}

void loop() {
//  client.connected();
  // update uv and soil
  percent = readSoil();
  light = uv.index();
  if (light > 5){
    high_light = true; // if we have light idx over 5, it is too much
  }
  if (light <=5 && light >=2){
    light_time = millis();
  }
  if (millis() - light_time >86400000){
    low_light = true; // if we do not get enough light over 24 h, warn start
  }
  if (percent < 0.7){
    low_water = true; //if water < 70%, warn
  }
  low_water = true;
  while(high_light || low_light || low_water){
    if (high_light){
      sprintf(var,"high_light");
      high_light = false;
    }
    else if(low_light){
      sprintf(var,"low_light");
      low_light = false;
    }
    else{
      sprintf(var,"low_water");
      low_water = false;
    }
  // set up WIFI module
    Serial.println();
    if (!client.connected()){
      Serial.println("Starting connection to server...");
      client.connect(server, 5000); 
      }
    Serial.println("Connected to server");
    // Make a HTTP request
    sprintf(get_request,"GET /?var=%s HTTP/1.1\r\nHost: 18.221.147.67\r\nConnection: close\r\n\r\n", var);
    client.print(get_request);
    delay(500);
    while (client.available()) {
      char c = client.read();
      Serial.write(c); 
      }
    delay(20000); 
  }
}

// extra function for soil moisture
float readSoil()
{
    digitalWrite(soilPower, HIGH);//turn D7 "On"
    delay(10);//wait 10 milliseconds 
    value = analogRead(soilPin);//Read the SIG value form sensor 
    digitalWrite(soilPower, LOW);//turn D7 "Off"
    return value/880;//send current moisture value presentage
}

// extra function for WIFI module
//void printWifiStatus() {
//  Serial.println(WiFi.SSID());
//  IPAddress ip = WiFi.localIP(); 
//  Serial.print("IP Address: "); 
//  Serial.println(ip);
//  long rssi = WiFi.RSSI(); 
//  Serial.print("Signal strength (RSSI):"); 
//  Serial.print(rssi);
//  Serial.println(" dBm"); 
//  }
