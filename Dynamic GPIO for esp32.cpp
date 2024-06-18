#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

const char* ssid = "SENSORFLOW";
const char* password = "12345678";

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  // Route to set GPIO pin state
  server.on("/setgpio", HTTP_GET, [](AsyncWebServerRequest *request){
    DynamicJsonDocument jsonResponse(1024);
    
    if (request->hasParam("gpio") && request->hasParam("state")) {
      int gpio = request->getParam("gpio")->value().toInt();
      String state = request->getParam("state")->value();

      if (gpio >= 0 && gpio <= 33) { // Assuming GPIO 0-33 for ESP32
        pinMode(gpio, OUTPUT); // Set the pin mode dynamically
        if (state == "high") {
          digitalWrite(gpio, HIGH);
          jsonResponse["gpio"] = gpio;
          jsonResponse["state"] = "HIGH";
          jsonResponse["status"] = "success";
          String response;
          serializeJson(jsonResponse, response);
          request->send(200, "application/json", response);
        } else if (state == "low") {
          digitalWrite(gpio, LOW);
          jsonResponse["gpio"] = gpio;
          jsonResponse["state"] = "LOW";
          jsonResponse["status"] = "success";
          String response;
          serializeJson(jsonResponse, response);
          request->send(200, "application/json", response);
        } else if (state.startsWith("pwm")) {
          int pwmValue = state.substring(3).toInt();
          ledcAttachPin(gpio, gpio); // Attach PWM to the pin
          ledcSetup(gpio, 5000, 8); // 5 kHz PWM with 8-bit resolution
          ledcWrite(gpio, pwmValue);
          jsonResponse["gpio"] = gpio;
          jsonResponse["state"] = "PWM";
          jsonResponse["pwm_value"] = pwmValue;
          jsonResponse["status"] = "success";
          String response;
          serializeJson(jsonResponse, response);
          request->send(200, "application/json", response);
        } else {
          jsonResponse["error"] = "Invalid state value";
          jsonResponse["status"] = "failure";
          String response;
          serializeJson(jsonResponse, response);
          request->send(400, "application/json", response);
        }
      } else {
        jsonResponse["error"] = "Invalid GPIO pin";
        jsonResponse["status"] = "failure";
        String response;
        serializeJson(jsonResponse, response);
        request->send(400, "application/json", response);
      }
    } else {
      jsonResponse["error"] = "GPIO or state parameter missing";
      jsonResponse["status"] = "failure";
      String response;
      serializeJson(jsonResponse, response);
      request->send(400, "application/json", response);
    }
  });

  // Start server
  server.begin();
}

void loop() {
  // Nothing needed here
}
