
# ESP32 GPIO Controller

The ESP32 GPIO Controller is a project that allows you to control the GPIO pins of an ESP32 via HTTP API calls. You can set the GPIO pins to HIGH, LOW, or PWM values using simple HTTP requests.

## Features

- Control any GPIO pin on the ESP32 dynamically
- Set GPIO pins to HIGH or LOW
- Set PWM values on GPIO pins
- Batch operations to control multiple pins in one request
- Read the current state of GPIO pins
- Schedule operations with optional duration

## Requirements

- ESP32 development board
- Arduino IDE or PlatformIO
- Wi-Fi network

## Libraries

Make sure you have the following libraries installed in your Arduino IDE:

- `WiFi.h`
- `ESPAsyncWebServer.h`
- `AsyncTCP.h`
- `ArduinoJson.h`

You can install these libraries via the Library Manager in the Arduino IDE.

## Wiring

Connect your ESP32 to your computer via USB. No additional wiring is required for this project unless you want to connect external components to the GPIO pins.

## Installation

1. Clone this repository or download the ZIP file and extract it.
2. Open the project in the Arduino IDE or PlatformIO.
3. Modify the `ssid` and `password` variables in the code to match your Wi-Fi network credentials.
4. Upload the code to your ESP32.

## Usage

Once the ESP32 is connected to your Wi-Fi network, it will print its IP address to the Serial Monitor. Use this IP address to send HTTP requests to control the GPIO pins.

### Example URLs

- **Set GPIO 4 to HIGH:**
  ```
  http://192.168.1.100:8080/setgpio?gpio=4&state=high
  ```

- **Set GPIO 4 to LOW:**
  ```
  http://192.168.1.100:8080/setgpio?gpio=4&state=low
  ```

- **Set GPIO 4 to PWM with a value of 128:**
  ```
  http://192.168.1.100:8080/setgpio?gpio=4&state=pwm128
  ```

- **Batch Operations:**
  ```
  http://192.168.1.100:8080/batch?operations=[{"gpio":4,"state":"high"},{"gpio":5,"state":"low"}]
  ```

- **Read GPIO 4 State:**
  ```
  http://192.168.1.100:8080/readgpio?gpio=4
  ```

- **Schedule GPIO 4 to HIGH after 30 seconds:**
  ```
  http://192.168.1.100:8080/schedule?gpio=4&state=high&delay=30000
  ```

- **Schedule GPIO 4 to HIGH after 30 seconds for 10 seconds:**
  ```
  http://192.168.1.100:8080/schedule?gpio=4&state=high&delay=30000&duration=10000
  ```

Replace `192.168.1.100` with the actual IP address of your ESP32.

## API Endpoints

### `/setgpio`

**Parameters:**

- `gpio`: The GPIO pin number to control (0-33 for ESP32).
- `state`: The state to set for the GPIO pin. Valid values are `high`, `low`, or `pwm<value>` (e.g., `pwm128` for a PWM value of 128).

**Response:**

- `200 OK`: If the request is successful.
  ```json
  {
    "gpio": 4,
    "state": "HIGH",
    "status": "success"
  }
  ```
- `400 Bad Request`: If the request parameters are missing or invalid.
  ```json
  {
    "error": "Invalid GPIO pin",
    "status": "failure"
  }
  ```

### `/batch`

**Parameters:**

- `operations`: A JSON array of operations, each with `gpio` and `state`.

**Example URL:**

```
http://192.168.1.100:8080/batch?operations=[{"gpio":4,"state":"high"},{"gpio":5,"state":"low"}]
```

**Response:**

- `200 OK`: If the request is successful.
  ```json
  {
    "status": "success"
  }
  ```
- `400 Bad Request`: If the request parameters are missing or invalid.
  ```json
  {
    "error": "operations parameter missing",
    "status": "failure"
  }
  ```

### `/readgpio`

**Parameters:**

- `gpio`: The GPIO pin number to read (0-33 for ESP32).

**Example URL:**

``    http://192.168.1.100:8080/readgpio?gpio=4
```

**Response:**

- `200 OK`: If the request is successful.
  ```json
 

 {
    "gpio": 4,
    "state": "HIGH"
  }
  ```
- `400 Bad Request`: If the request parameters are missing or invalid.
  ```json
  {
    "error": "gpio parameter missing",
    "status": "failure"
  }
  ```

### `/schedule`

**Parameters:**

- `gpio`: The GPIO pin number to control (0-33 for ESP32).
- `state`: The state to set for the GPIO pin. Valid values are `high`, `low`, or `pwm<value>` (e.g., `pwm128` for a PWM value of 128).
- `delay`: The delay in milliseconds before the operation is executed.
- `duration` (optional): The duration in milliseconds for which the pin should stay in the specified state before resetting to LOW (for HIGH and PWM states only).

**Example URLs:**

- Schedule GPIO 4 to HIGH after 30 seconds:
  ```
  http://192.168.1.100:8080/schedule?gpio=4&state=high&delay=30000
  ```

- Schedule GPIO 4 to HIGH after 30 seconds for 10 seconds:
  ```
  http://192.168.1.100:8080/schedule?gpio=4&state=high&delay=30000&duration=10000
  ```

**Response:**

- `200 OK`: If the request is successful.
  ```json
  {
    "status": "scheduled"
  }
  ```
- `400 Bad Request`: If the request parameters are missing or invalid.
  ```json
  {
    "error": "gpio, state, or delay parameter missing",
    "status": "failure"
  }
  ```

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Acknowledgments

This project uses the ESPAsyncWebServer and ArduinoJson libraries for handling HTTP requests efficiently and formatting responses as JSON.

---

Feel free to contribute to this project by submitting issues or pull requests.
```

This updated code and README file should provide you with the desired functionality and error handling.
