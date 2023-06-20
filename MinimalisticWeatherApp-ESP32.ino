#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "MQ135.h"
#include <Arduino.h>

const char* ssid = "esp32";
const char* password = "12345678";

WebServer server(80);
int selectedNetworkIndex = -1;
String enteredPassword;

//SENSORS

#define MQ135_ANALOG_PIN 34
#define UV_PIN 32

int uv_Index;
int uv_temp;

//BME280
Adafruit_BME280 bme;

MQ135 gasSensor = MQ135(MQ135_ANALOG_PIN);

//DUST SENSOR
int measurePin = 35;
int ledPower = 23;

int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;

float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;


void setup() {
  Serial.begin(115200);

  // set up the dust sensor
  pinMode(ledPower,OUTPUT);
  pinMode(UV_PIN, INPUT);

  while (!Serial) {
    delay(1);
  }

  WiFi.mode(WIFI_STA);
  int n = WiFi.scanNetworks();

  Serial.println("Available WiFi Networks:");
  for (int i = 0; i < n; ++i) {
    Serial.println(WiFi.SSID(i));
  }

  WiFi.softAP(ssid, password);

  Serial.println("Access point started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/connect", HTTP_POST, handleConnect);
  server.on("/password", HTTP_GET, handlePassword);
  server.on("/submit", HTTP_POST, handleSubmit);
  server.on("/success", HTTP_GET, handleSuccess);
  server.on("/failed", HTTP_GET, handleFailed);

  //sensors
  server.on("/data", HTTP_GET, handleData);

  server.begin();

  if (!bme.begin(0x76)) {  
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
}

void loop() {
  server.handleClient();

  int uv_temp = map(analogRead(UV_PIN), 0, 1023, 0, 1200);

  if(uv_temp < 227){
    uv_Index = 0;
  } else if(uv_temp < 318){
    uv_Index = 1;
  } else if(uv_temp < 408){
    uv_Index = 2;
  } else if(uv_temp < 503){
    uv_Index = 3;
  } else if(uv_temp < 606){
    uv_Index = 4;
  } else if(uv_temp < 696){
    uv_Index = 5;
  } else if(uv_temp < 795){
    uv_Index = 6;
  } else if(uv_temp < 881){
    uv_Index = 7;
  } else if(uv_temp < 976){
    uv_Index = 8;
  } else if(uv_temp < 1079){
    uv_Index = 9;
  } else if(uv_temp < 1170){
    uv_Index = 10;
  } else {
    uv_Index = 11;
  }

  digitalWrite(ledPower,LOW);
  delayMicroseconds(samplingTime);

  voMeasured = analogRead(measurePin);

  delayMicroseconds(deltaTime);
  digitalWrite(ledPower,HIGH);
  delayMicroseconds(sleepTime);

  calcVoltage = voMeasured * (5.0 / 1024.0);

  dustDensity = 170 * calcVoltage - 0.1;

  delay(1000);
}

void handleRoot() {
  String html = "<!DOCTYPE html>";
  html += "<html>";
  html += "<head>";
  html += "<title>Wi-Fi Selection</title>";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<style>";
  html += "body {";
  html += "    font-family: Arial, sans-serif;";
  html += "    margin: 0;";
  html += "    padding: 20px;";
  html += "    background-color: #f2f2f2;";
  html += "}";
  html += "h1 {";
  html += "    text-align: center;";
  html += "    color: #333;";
  html += "}";
  html += "form {";
  html += "    max-width: 300px;";
  html += "    margin: auto;";
  html += "    background-color: #fff;";
  html += "    padding: 20px;";
  html += "    border-radius: 5px;";
  html += "    box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);";
  html += "}";
  html += "label {";
  html += "    display: block;";
  html += "    margin-bottom: 10px;";
  html += "    color: #333;";
  html += "}";
  html += "input[type=\"text\"],";
  html += "input[type=\"password\"] {";
  html += "    width: 100%;";
  html += "    padding: 10px;";
  html += "    border: 1px solid #ccc;";
  html += "    border-radius: 4px;";
  html += "    box-sizing: border-box;";
  html += "}";
  html += "input[type=\"submit\"] {";
  html += "    background-color: #4CAF50;";
  html += "    color: white;";
  html += "    border: none;";
  html += "    padding: 10px 20px;";
  html += "    text-align: center;";
  html += "    text-decoration: none;";
  html += "    display: inline-block;";
  html += "    font-size: 16px;";
  html += "    border-radius: 4px;";
  html += "    cursor: pointer;";
  html += "}";
  html += "</style>";
  html += "</head>";
  html += "<body>";
  html += "<h1>Select Wi-Fi Network</h1>";
  html += "<form action=\"/connect\" method=\"POST\">";
  html += "    <label for=\"network\">Wi-Fi Network:</label>";
  html += "    <select name=\"network\">";
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    html += "        <option value=\"" + String(i) + "\">" + WiFi.SSID(i) + "</option>";
  }
  html += "    </select>";
  html += "    <input type=\"submit\" value=\"Connect\">";
  html += "</form>";
  html += "</body>";
  html += "</html>";

  server.send(200, "text/html", html);
}

void handleConnect() {
  if (server.args() > 0) {
    for (size_t i = 0; i < server.args(); i++) {
      if (server.argName(i) == "network") {
        selectedNetworkIndex = server.arg(i).toInt();
        break;
      }
    }
  }

  server.sendHeader("Location", "/password");
  server.send(302);
}

void handlePassword() {
  String html = "<!DOCTYPE html>";
  html += "<html>";
  html += "<head>";
  html += "<title>Wi-Fi Password</title>";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<style>";
  html += "body {";
  html += "    font-family: Arial, sans-serif;";
  html += "    margin: 0;";
  html += "    padding: 20px;";
  html += "    background-color: #f2f2f2;";
  html += "}";
  html += "h1 {";
  html += "    text-align: center;";
  html += "    color: #333;";
  html += "}";
  html += "form {";
  html += "    max-width: 300px;";
  html += "    margin: auto;";
  html += "    background-color: #fff;";
  html += "    padding: 20px;";
  html += "    border-radius: 5px;";
  html += "    box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);";
  html += "}";
  html += "label {";
  html += "    display: block;";
  html += "    margin-bottom: 10px;";
  html += "    color: #333;";
  html += "}";
  html += "input[type=\"password\"] {";
  html += "    width: 100%;";
  html += "    padding: 10px;";
  html += "    border: 1px solid #ccc;";
  html += "    border-radius: 4px;";
  html += "    box-sizing: border-box;";
  html += "}";
  html += "input[type=\"submit\"] {";
  html += "    background-color: #4CAF50;";
  html += "    color: white;";
  html += "    border: none;";
  html += "    padding: 10px 20px;";
  html += "    text-align: center;";
  html += "    text-decoration: none;";
  html += "    display: block;";
  html += "    margin: 20px auto;";
  html += "    font-size: 16px;";
  html += "    border-radius: 4px;";
  html += "    cursor: pointer;";
  html += "}";
  html += "</style>";
  html += "</head>";
  html += "<body>";
  html += "<h1>Enter Wi-Fi Password</h1>";
  html += "<form action=\"/submit\" method=\"POST\">";
  html += "    <label for=\"password\">Wi-Fi Password:</label>";
  html += "    <input type=\"password\" id=\"password\" name=\"password\" style=\"margin-top: 10px;\">";
  html += "    <input type=\"submit\" value=\"Submit\">";
  html += "</form>";
  html += "</body>";
  html += "</html>";

  server.send(200, "text/html", html);
}

void handleSubmit() {
  if (server.args() > 0) {
    for (size_t i = 0; i < server.args(); i++) {
      if (server.argName(i) == "password") {
        enteredPassword = server.arg(i);
        break;
      }
    }
  }

  if (selectedNetworkIndex >= 0 && selectedNetworkIndex < WiFi.scanNetworks()) {
    String ssid = WiFi.SSID(selectedNetworkIndex);

    WiFi.begin(ssid.c_str(), enteredPassword.c_str());

    unsigned long timeout = millis() + 10000;

    while (WiFi.status() != WL_CONNECTED) {
      if (millis() > timeout) {
        Serial.println("Failed to connect to " + ssid);
        break;
      }
      delay(1000);
      Serial.println("Connecting to WiFi...");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected to " + ssid);
      Serial.print("Assigned IP address: ");
      Serial.println(WiFi.localIP());
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    server.sendHeader("Location", "/success");
    server.send(302);
    return;
  } else {
    server.sendHeader("Location", "/failed");
    server.send(302);
    return;
  }

  server.sendHeader("Location", "/");
  server.send(302);
}

void handleSuccess() {
  String html = "<!DOCTYPE html>";
  html += "<html>";
  html += "<head>";
  html += "<title>Connection Successful</title>";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<style>";
  html += "body {";
  html += "    font-family: Arial, sans-serif;";
  html += "    margin: 0;";
  html += "    padding: 20px;";
  html += "    background-color: #f2f2f2;";
  html += "}";
  html += "h1 {";
  html += "    text-align: center;";
  html += "    color: #333;";
  html += "}";
  html += "p {";
  html += "    text-align: center;";
  html += "    color: #666;";
  html += "}";
  html += "button {";
  html += "    display: block;";
  html += "    width: 100px;";
  html += "    height: 25px;";
  html += "    margin: 20px auto;";
  html += "    background-color: #4CAF50;";
  html += "    color: white;";
  html += "    text-align: center;";
  html += "    text-decoration: none;";
  html += "    display: inline-block;";
  html += "    font-size: 16px;";
  html += "    border: none;";
  html += "}";
  html += "</style>";
  html += "</head>";
  html += "<body>";
  html += "<h1>Connection Successful!</h1>";
  html += "<p>You are now connected to the WiFi network.</p>";
  html += "<button onclick=\"location.href='/'\">Back</button>";
  html += "</body>";
  html += "</html>";

  server.send(200, "text/html", html);

  server.send(200, "text/html", html);
}

void handleFailed() {
  String html = "<!DOCTYPE html>";
  html += "<html>";
  html += "<head>";
  html += "<title>Invalid password</title>";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<style>";
  html += "body {";
  html += "    font-family: Arial, sans-serif;";
  html += "    margin: 0;";
  html += "    padding: 20px;";
  html += "    background-color: #f2f2f2;";
  html += "}";
  html += "h1 {";
  html += "    text-align: center;";
  html += "    color: #333;";
  html += "}";
  html += "p {";
  html += "    text-align: center;";
  html += "    color: #666;";
  html += "}";
  html += "button {";
  html += "    display: block;";
  html += "    width: 100px;";
  html += "    height: 25px;";
  html += "    margin: 20px auto;";
  html += "    background-color: #f44336;";
  html += "    color: white;";
  html += "    text-align: center;";
  html += "    text-decoration: none;";
  html += "    display: inline-block;";
  html += "    font-size: 16px;";
  html += "    border: none;";
  html += "}";
  html += "</style>";
  html += "</head>";
  html += "<body>";
  html += "<h1>Invalid password!</h1>";
  html += "<p>Unable to connect to the WiFi network.</p>";
  html += "<button onclick=\"location.href='/'\">Back</button>";
  html += "</body>";
  html += "</html>";

  server.send(200, "text/html", html);
}


void handleData() {
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;

  String json = "{";
  json += "\"temperature\":" + String(temperature) + ",";
  json += "\"humidity\":" + String(humidity) + ",";
  json += "\"pressure\":" + String(pressure) + ",";
  json += "\"totalgasppm\":" + String(gasSensor.getPPM()) + ",";
  json += "\"dustDensity\":" + String(dustDensity) + ",";
  json += "\"uvindex\":" + String(uv_Index);
  json += "}";

  server.send(200, "application/json", json);
}
