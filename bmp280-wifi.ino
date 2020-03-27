/*********
  NodeMCU w BMP280 and webserver for temp&pressure 
********/

#include <Arduino.h>
//#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>
#include <string>


#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;
Adafruit_BMP280 bmp; // use I2C interface
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();

float temperature, humidity, pressure, altitude;

// Replace with your network credentials 
const char* ssid = "TR";  // Enter SSID here
const char* password = "medved28";  //Enter Password here

//ESP8266WebServer server(80);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


//==========================the HTML code====================

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
  <title>ESP8266+BMP280</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .ds-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h6>ESP8266+BMP280 ©Dr.András Szép</h6>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels"> </span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units"></sup>&deg;C
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels"> </span>
    <span id="pressure">%PRESSURE%</span>
    <sup class="units"></sup>mBar
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("pressure").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/pressure", true);
  xhttp.send();
}, 10000) ;
</script>
</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return String(temperature);
  }
  else if(var == "PRESSURE"){
    return String(pressure);
  }
  return String();
}

//========================SETUP=================================================

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.println("\n=============\nTemperature and pressure monitor by Dr.András Szép ©2020\n");
  
//  Wire.begin(); // initialise I2C protocol
  
//  bmp.begin(0x76);   
  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  bmp_temp->printSensorDetails();
  bmp_pressure->printSensorDetails();
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("BMP WiFi connecting to: ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // Print ESP Local IP Address
  Serial.print("\nConnected IP:");
  Serial.println(WiFi.localIP());
 
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){ 
    request->send_P(200, "text/plain", String(temperature).c_str() );
  });
  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request){ 
    request->send_P(200, "text/plain", String(pressure).c_str());
  });
  
  // Start server
  server.begin();  
}
 
void loop(){
  temperature = bmp.readTemperature();
  pressure = bmp.readPressure() / 100.0F;
  altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA);

  Serial.print("Atmospheric pressure = ");
  Serial.print(pressure,2); // print with 2 decimal places
  Serial.print(" hPa,  Temperature = ");
  Serial.print(temperature,1); // print with 1 decimal places
  Serial.print( " ˚C,   Altitude = ");
  Serial.print(altitude,1); // print with 1 decimal places
  Serial.println( " m a.s.l");
  delay(5000);
}
