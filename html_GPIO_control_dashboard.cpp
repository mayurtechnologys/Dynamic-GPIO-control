#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <Preferences.h>

const char* ssid = "SENSORFLOW";
const char* password = "12345678";

AsyncWebServer server(80);
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
    .operation {
      margin: 20px;
    }
  </style>
</head>
<body>
  <h1>ESP32 GPIO Control Dashboard</h1>
  <div id="gpio-controls"></div>
  <div class="operation">
    <h3>Schedule Operation</h3>
    GPIO: <input type="number" id="schedule-gpio" placeholder="GPIO">
    State: <input type="text" id="schedule-state" placeholder="high/low/pwm">
    Delay (ms): <input type="number" id="schedule-delay" placeholder="Delay in ms">
    Duration (ms): <input type="number" id="schedule-duration" placeholder="Duration in ms">
    <button onclick="scheduleOperation()">Schedule</button>
  </div>
  <div class="operation">
    <h3>Batch Operation</h3>
    Operations (JSON): <textarea id="batch-operations" rows="4" cols="50" placeholder='[{"gpio": 2, "state": "high"}, {"gpio": 3, "state": "low"}]'></textarea>
    <button onclick="batchOperation()">Execute Batch</button>
  </div>

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

    function scheduleOperation() {
      const gpio = document.getElementById('schedule-gpio').value;
      const state = document.getElementById('schedule-state').value;
      const delay = document.getElementById('schedule-delay').value;
      const duration = document.getElementById('schedule-duration').value;
      fetch(`/schedule?gpio=${gpio}&state=${state}&delay=${delay}&duration=${duration}`)
        .then(response => response.json())
        .then(data => {
          if (data.status === 'scheduled') {
            alert(`Scheduled GPIO ${gpio} to ${state.toUpperCase()} after ${delay}ms for ${duration}ms`);
          } else {
            alert(`Failed to schedule GPIO ${gpio}: ${data.error}`);
          }
        });
    }

    function batchOperation() {
      const operations = document.getElementById('batch-operations').value;
      fetch(`/batch?operations=${encodeURIComponent(operations)}`)
        .then(response => response.json())
        .then(data => {
          if (data.status === 'success') {
            alert(`Batch operations executed successfully`);
          } else {
            alert(`Failed to execute batch operations: ${data.error}`);
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

  // Schedule Operation
  server.on("/schedule", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("gpio") && request->hasParam("state") && request->hasParam("delay")) {
      operationArgs.gpio = request->getParam("gpio")->value().toInt();
      operationArgs.state = request->getParam("state")->value();
      int delayMs = request->getParam("delay")->value().toInt();
      operationArgs.duration = request->hasParam("duration") ? request->getParam("duration")->value().toInt() : 0;

      Serial.print("Scheduling operation: GPIO=");
      Serial.print(operationArgs.gpio);
      Serial.print(", State=");
      Serial.print(operationArgs.state);
      Serial.print(", Delay=");
      Serial.print(delayMs);
      Serial.print(", Duration=");
      Serial.println(operationArgs.duration);

      scheduler.once_ms(delayMs, scheduleOperation);
      request->send(200, "application/json", "{\"status\":\"scheduled\"}");
    } else {
      request->send(400, "application/json", "{\"error\":\"gpio, state, or delay parameter missing\",\"status\":\"failure\"}");
    }
  });

  // Batch Operation
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

        // Store the operation state
        preferences.begin("gpio-states", false);
        preferences.putString(String(gpio).c_str(), state);
        preferences.end();
      }
      request->send(200, "application/json", "{\"status\":\"success\"}");
    } else {
      request->send(400, "application/json", "{\"error\":\"operations parameter missing\",\"status\":\"failure\"}");
    }
  });

  // Start server
  server.begin();
  Serial.println("Server started...");
}

void loop() {
  // Nothing needed here
}
