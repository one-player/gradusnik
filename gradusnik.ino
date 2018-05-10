/*
Connecting the BME280 Sensor:
Sensor              ->  Board
-----------------------------
Vin (Voltage In)    ->  3.3V
Gnd (Ground)        ->  Gnd
SDA (Serial Data)   ->  A4/GPIO-0 on Uno/Pro-Mini, 20 on Mega2560/Due, 2 Leonardo/Pro-Micro
SCK (Serial Clock)  ->  A5/GPIO-2 on Uno/Pro-Mini, 21 on Mega2560/Due, 3 Leonardo/Pro-Micro
 */
#include <BME280I2C.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

/*WiFi prop.*/
const char* ssid = "Kraken";
const char* password = "kfgrfkfgrf1987";
const char* host = "192.168.0.14";//IP принимающего сервера
const int httpPort = 80;
/*WiFi prop.*/

float temp_tmp;
float hum_tmp;
float pres_tmp;

unsigned long times;

BME280I2C bme;
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
WiFiClient client;

void setup() {
  Serial.begin(9600);
  Wire.begin(4,5);
  while(!Serial) {} // Wait
  while(!bme.begin()){
    Serial.println("Could not find BME280 sensor!");
    delay(1000);
  } 
  switch(bme.chipModel())
  {
     case BME280::ChipModel_BME280:
       Serial.println("Found BME280 sensor! Success.");
       break;
     case BME280::ChipModel_BMP280:
       Serial.println("Found BMP280 sensor! No Humidity available.");
       break;
     default:
       Serial.println("Found UNKNOWN sensor! Error!");
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
  Serial.printf("Web server started, open %s in a web browser\n", WiFi.localIP().toString().c_str());
  MDNS.begin("Gradusnik");
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);//HTTPUpdateServer open http://IP/update
}

void loop() {
  httpServer.handleClient();
  if (millis()-times>1800000){
    times=millis();
    WiFiClient client;
    if (!client.connect(host, httpPort)) 
      {
        Serial.println("connection failed");
        return;
      }
    float temp(NAN), hum(NAN), pres(NAN);
    BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
    BME280::PresUnit presUnit(BME280::PresUnit_Pa);
    bme.read(pres, temp, hum, tempUnit, presUnit);
    client.print(String("GET /insert_param.php?temp=")+temp+"&hum="+hum+"&pres="+pres+" HTTP/1.1\r\nHost: "+host+"\r\nConnection: close\r\n\r\n");
    client.stop();                                      // Закрытие соединения
  }
}
