#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "esp32";
const char* password = "12345678";

WebServer server(80);
int selectedNetworkIndex = -1;
String enteredPassword;

void setup() {
  Serial.begin(115200);

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

  server.begin();
}

void loop() {
  server.handleClient();
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
      // Additional actions after successful connection can be added here
    }
  }

  server.sendHeader("Location", "/");
  server.send(302);
}
