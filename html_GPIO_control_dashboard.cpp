#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <Preferences.h>

const char* ssid = "SENSORFLOW";
const char* password = "12345678";

AsyncWebServer server(80); // Change to default port 80 for web
Ticker scheduler;
Ticker resetScheduler;
Preferences preferences;

struct OperationArgs {
  int gpio;
  String state;
  int duration;
};

OperationArgs operationArgs;

// Function prototypes
void resetOperation();
void scheduleOperation();

// HTML content for the dashboard
const char* html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 GPIO Control</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
    }
    .gpio-control {
      display: inline-block;
      margin: 20px;
    }
    .gpio-control button {
      width: 100px;
      height: 50px;
      margin: 5px;
    }
  </style>
</head>
<body>
  <h1>ESP32 GPIO Control Dashboard</h1>
  <div id="gpio-controls"></div>

  <script>
    const gpioPins = [...Array(34).keys()]; // GPIO 0 to 33

    function createControlElement(gpio) {
      const container = document.createElement('div');
      container.className = 'gpio-control';

      const label = document.createElement('h3');
      label.innerText = `GPIO ${gpio}`;
      container.appendChild(label);

      const highButton = document.createElement('button');
      highButton.innerText = 'HIGH';
      highButton.onclick = () => setGPIOState(gpio, 'high');
      container.appendChild(highButton);

      const lowButton = document.createElement('button');
      lowButton.innerText = 'LOW';
      lowButton.onclick = () => setGPIOState(gpio, 'low');
      container.appendChild(lowButton);

      const pwmInput = document.createElement('input');
      pwmInput.type = 'number';
      pwmInput.placeholder = 'PWM value';
      pwmInput.onchange = () => setGPIOState(gpio, `pwm${pwmInput.value}`);
      container.appendChild(pwmInput);

      return container;
    }

    function setGPIOState(gpio, state) {
      fetch(`/setgpio?gpio=${gpio}&state=${state}`)
        .then(response => response.json())
        .then(data => {
          if (data.status === 'success') {
            alert(`GPIO ${gpio} set to ${state.toUpperCase()}`);
          } else {
            alert(`Failed to set GPIO ${gpio}: ${data.error}`);
          }
        });
    }

    function init() {
      const gpioControls = document.getElementById('gpio-controls');
      gpioPins.forEach(gpio => {
        const controlElement = createControlElement(gpio);
        gpioControls.appendChild(controlElement);
      });
    }

    window.onload = init;
  </script>
</body>
</html>
)rawliteral";

void resetOperation() {
  Serial.println("Resetting operation...");
  if (operationArgs.state == "high" || operationArgs.state.startsWith("pwm")) {
    digitalWrite(operationArgs.gpio, LOW); // Reset pin to LOW after duration
    Serial.print("GPIO ");
    Serial.print(operationArgs.gpio);
    Serial.println(" set to LOW");
  } else if (operationArgs.state == "low") {
    digitalWrite(operationArgs.gpio, HIGH); // Reset pin to HIGH after duration
    Serial.print("GPIO ");
    Serial.print(operationArgs.gpio);
    Serial.println(" set to HIGH");
  }
  // Store the reset state
  preferences.begin("gpio-states", false);
  preferences.remove(String(operationArgs.gpio).c_str());
  preferences.end();
}

void scheduleOperation() {
  Serial.print("Scheduling operation on GPIO ");
  Serial.print(operationArgs.gpio);
  Serial.print(" with state ");
  Serial.println(operationArgs.state);
  
  pinMode(operationArgs.gpio, OUTPUT);
  if (operationArgs.state == "high") {
    digitalWrite(operationArgs.gpio, HIGH);
    Serial.print("GPIO ");
    Serial.print(operationArgs.gpio);
    Serial.println(" set to HIGH");
  } else if (operationArgs.state == "low") {
    digitalWrite(operationArgs.gpio, LOW);
    Serial.print("GPIO ");
    Serial.print(operationArgs.gpio);
    Serial.println(" set to LOW");
  } else if (operationArgs.state.startsWith("pwm")) {
    int pwmValue = operationArgs.state.substring(3).toInt();
    ledcAttachPin(operationArgs.gpio, operationArgs.gpio);
    ledcSetup(operationArgs.gpio, 5000, 8);
    ledcWrite(operationArgs.gpio, pwmValue);
    Serial.print("GPIO ");
    Serial.print(operationArgs.gpio);
    Serial.print(" set to PWM with value ");
    Serial.println(pwmValue);
  }

  if (operationArgs.duration > 0) {
    resetScheduler.once_ms(operationArgs.duration, resetOperation); // Schedule reset after duration
  }

  // Store the operation state
  preferences.begin("gpio-states", false);
  preferences.putString(String(operationArgs.gpio).c_str(), operationArgs.state);
  preferences.end();
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting setup...");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  // Resume previous GPIO states
  preferences.begin("gpio-states", true);
  for (int i = 0; i <= 33; i++) {
    String state = preferences.getString(String(i).c_str(), "");
    if (state != "") {
      Serial.print("Resuming state for GPIO ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(state);
      
      operationArgs.gpio = i;
      operationArgs.state = state;
      operationArgs.duration = 0; // Set to 0 to avoid scheduling reset
      scheduleOperation();
    } else {
      Serial.print("No state found for GPIO ");
      Serial.println(i);
    }
  }
  preferences.end();

  // Serve the HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", html);
  });

  // Set GPIO
  server.on("/setgpio", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("gpio") && request->hasParam("state")) {
      int gpio = request->getParam("gpio")->value().toInt();
      String state = request->getParam("state")->value();

      if (gpio >= 0 && gpio <= 33) { // Assuming GPIO 0-33 for ESP32
        pinMode(gpio, OUTPUT); // Set the pin mode dynamically
        if (state == "high") {
          digitalWrite(gpio, HIGH);
          DynamicJsonDocument jsonResponse(1024);
          jsonResponse["gpio"] = gpio;
          jsonResponse["state"] = "HIGH";
          jsonResponse["status"] = "success";
          String response;
          serializeJson(jsonResponse, response);
          request->send(200, "application/json", response);
        } else if (state == "low") {
          digitalWrite(gpio, LOW);
          DynamicJsonDocument jsonResponse(1024);
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
          DynamicJsonDocument jsonResponse(1024);
          jsonResponse["gpio"] = gpio;
          jsonResponse["state"] = "PWM";
          jsonResponse["pwm_value"] = pwmValue;
          jsonResponse["status"] = "success";
          String response;
          serializeJson(jsonResponse, response);
          request->send(200, "application/json", response);
        } else {
          request->send(400, "application/json", "{\"error\":\"Invalid state value\",\"status\":\"failure\"}");
        }

        // Store the operation state
        preferences.begin("gpio-states", false);
        preferences.putString(String(gpio).c_str(), state);
        preferences.end();

      } else {
        request->send(400, "application/json", "{\"error\":\"Invalid GPIO pin\",\"status\":\"failure\"}");
      }
    } else {
      request->send(400, "application/json", "{\"error\":\"GPIO or state parameter missing\",\"status\":\"failure\"}");
    }
  });

  // Start server
  server.begin();
  Serial.println("Server started...");
}

void loop() {
  // Nothing needed here
}
