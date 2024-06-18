#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Ticker.h>

const char* ssid = "SENSORFLOW";
const char* password = "12345678";

AsyncWebServer server(80);
Ticker scheduler;

struct OperationArgs {
  int gpio;
  String state;
  int duration;
};

OperationArgs operationArgs;

void scheduleOperation() {
  pinMode(operationArgs.gpio, OUTPUT);
  if (operationArgs.state == "high") {
    digitalWrite(operationArgs.gpio, HIGH);
  } else if (operationArgs.state == "low") {
    digitalWrite(operationArgs.gpio, LOW);
  } else if (operationArgs.state.startsWith("pwm")) {
    int pwmValue = operationArgs.state.substring(3).toInt();
    ledcAttachPin(operationArgs.gpio, operationArgs.gpio);
    ledcSetup(operationArgs.gpio, 5000, 8);
    ledcWrite(operationArgs.gpio, pwmValue);
  }

  if (operationArgs.duration > 0) {
    delay(operationArgs.duration);
    digitalWrite(operationArgs.gpio, LOW); // Reset pin to LOW after duration
  }
}

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

  // Batch Operations
  server.on("/batch", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("operations")) {
      String operationsParam = request->getParam("operations")->value();
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, operationsParam);
      JsonArray operations = doc.as<JsonArray>();

      for (JsonVariant operation : operations) {
        int gpio = operation["gpio"];
        String state = operation["state"];
        pinMode(gpio, OUTPUT);
        if (state == "high") {
          digitalWrite(gpio, HIGH);
        } else if (state == "low") {
          digitalWrite(gpio, LOW);
        } else if (state.startsWith("pwm")) {
          int pwmValue = state.substring(3).toInt();
          ledcAttachPin(gpio, gpio);
          ledcSetup(gpio, 5000, 8);
          ledcWrite(gpio, pwmValue);
        }
      }
      request->send(200, "application/json", "{\"status\":\"success\"}");
    } else {
      request->send(400, "application/json", "{\"error\":\"operations parameter missing\"}");
    }
  });

  // Read GPIO State
  server.on("/readgpio", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("gpio")) {
      int gpio = request->getParam("gpio")->value().toInt();
      int state = digitalRead(gpio);
      DynamicJsonDocument jsonResponse(1024);
      jsonResponse["gpio"] = gpio;
      jsonResponse["state"] = state == HIGH ? "HIGH" : "LOW";
      String response;
      serializeJson(jsonResponse, response);
      request->send(200, "application/json", response);
    } else {
      request->send(400, "application/json", "{\"error\":\"gpio parameter missing\"}");
    }
  });

  // Schedule Operations
  server.on("/schedule", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("gpio") && request->hasParam("state") && request->hasParam("delay")) {
      operationArgs.gpio = request->getParam("gpio")->value().toInt();
      operationArgs.state = request->getParam("state")->value();
      int delayMs = request->getParam("delay")->value().toInt();
      operationArgs.duration = request->hasParam("duration") ? request->getParam("duration")->value().toInt() : 0;

      scheduler.once_ms(delayMs, scheduleOperation);
      request->send(200, "application/json", "{\"status\":\"scheduled\"}");
    } else {
      request->send(400, "application/json", "{\"error\":\"gpio, state, or delay parameter missing\"}");
    }
  });

  // Start server
  server.begin();
}

void loop() {
  // Nothing needed here
}
