#include <WiFi.h>
#include <ESPAsyncWebServer.h>

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
    if (request->hasParam("gpio") && request->hasParam("state")) {
      int gpio = request->getParam("gpio")->value().toInt();
      String state = request->getParam("state")->value();

      if (gpio >= 0 && gpio <= 33) { // Assuming GPIO 0-33 for ESP32
        pinMode(gpio, OUTPUT); // Set the pin mode dynamically
        if (state == "high") {
          digitalWrite(gpio, HIGH);
          request->send(200, "text/plain", "GPIO " + String(gpio) + " set to HIGH");
        } else if (state == "low") {
          digitalWrite(gpio, LOW);
          request->send(200, "text/plain", "GPIO " + String(gpio) + " set to LOW");
        } else if (state.startsWith("pwm")) {
          int pwmValue = state.substring(3).toInt();
          ledcAttachPin(gpio, gpio); // Attach PWM to the pin
          ledcSetup(gpio, 5000, 8); // 5 kHz PWM with 8-bit resolution
          ledcWrite(gpio, pwmValue);
          request->send(200, "text/plain", "GPIO " + String(gpio) + " set to PWM " + String(pwmValue));
        } else {
          request->send(400, "text/plain", "Invalid state value");
        }
      } else {
        request->send(400, "text/plain", "Invalid GPIO pin");
      }
    } else {
      request->send(400, "text/plain", "GPIO or state parameter missing");
    }
  });

  // Start server
  server.begin();
}

void loop() {
  // Nothing needed here
}
