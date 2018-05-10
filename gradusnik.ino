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
const char* ssid = "123";
const char* password = "k7";
const int httpPort = 80;
/*WiFi prop.*/

float temp_tmp;
float hum_tmp;
float pres_tmp;

//#define SERIAL_BAUD 9600
BME280I2C bme;
WiFiServer server(80);
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
  server.begin();
  Serial.printf("Web server started, open %s in a web browser\n", WiFi.localIP().toString().c_str());
  MDNS.begin("Gradusnik");
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);//HTTPUpdateServer open http://IP/update
}

void loop() {
  httpServer.handleClient();
  WiFiClient client = server.available();                // Получаем данные, посылаемые клиентом 
  if (client){
    boolean blank_line = true;                            // Создаем переменную, чтобы определить конец HTTP-запроса 
    while (client.connected()){                           // Пока есть соединение с клиентом 
      if (client.available()){                            // Если клиент активен 
        char c = client.read();                            // Считываем посылаемую информацию в переменную "с"
        if (c == '\n' && blank_line){                      // Вывод HTML страницы 
          
          float temp(NAN), hum(NAN), pres(NAN);
          BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
          BME280::PresUnit presUnit(BME280::PresUnit_Pa);
          bme.read(pres, temp, hum, tempUnit, presUnit);
          
          client.println("HTTP/1.1 200 OK");               // Стандартный заголовок HTTP 
          client.println("Content-Type: text/html; charset=utf-8"); 
          client.println("Connection: close");             // Соединение будет закрыто после завершения ответа
          client.println("Refresh: 10");                   // Автоматическое обновление каждые 10 сек 
          client.println();
          client.println("<!DOCTYPE HTML>");               // Веб-страница создается с использованием HTML
          client.println("<html>");                        // Открытие тега HTML 
          client.println("<head><style>body {background-color:#87CEEB; color:#ffffff; text-shadow: 0px 2px 6px rgba(0,0,0,0.4); font-family: Sans-Serif, Arial, Helvetica;}</style>");
          client.print("<title>Градусник</title>");     // Название страницы
          client.println("</head>");
          client.println("<body><center>");
          
          client.println("<div><h1>WiFi градусник</h1></div>"); 
          client.println("<h3>Температура = ");
          
          client.println(temp);      // Отображение температуры
          client.print("*C");
            if (temp_tmp<=temp)
            {
              client.println("<font color='green'> &uArr;</font></h3>");
              temp_tmp=temp;
            }else{
              client.println("<font color='red'> &dArr;</font></h3>");
              temp_tmp=temp;
            }          
          client.println("<h3>Влажность = ");
          client.println(hum);      // Отображение температуры
          client.print("% RH");
            if (hum_tmp<=hum)
            {
              client.println("<font color='green'> &uArr;</font></h3>");
              hum_tmp=hum;
            }else{
              client.println("<font color='red'> &dArr;</font></h3>");
              hum_tmp=hum;
            } 
          client.println("<h3>Давление = ");
          client.println(pres);      // Отображение температуры
          client.print(" atm");
          if (pres_tmp<=pres)
          {
            client.println("<font color='green'> &uArr;</font></h3>");
            pres_tmp=pres;
          }else{
            client.println("<font color='red'> &dArr;</font></h3>");
            pres_tmp=pres;
          }
          client.println("</center></body>");
          client.println("</html>");                       // Закрытие тега HTML 
          break;                                           // Выход
        }
          if (c == '\n'){                                 // Если "с" равен символу новой строки                                             
          blank_line = true;                             // Тогда начинаем новую строку
        }                                          
          else if (c != '\r'){                           // Если "с" не равен символу возврата курсора на начало строки                                        
          blank_line = false;                           // Тогда получаем символ на текущей строке 
        }                                        
      }
    }  
    client.stop();                                      // Закрытие соединения
    Serial.println("Client disconnected.");             // Печать "Клиент отключен"
  }
}
