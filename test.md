
### Example URLs for Testing

#### Set GPIO 2 to HIGH
```
http://192.168.235.110:8080/setgpio?gpio=2&state=high
```

#### Set GPIO 2 to LOW
```
http://192.168.235.110:8080/setgpio?gpio=2&state=low
```

#### Set GPIO 2 to PWM with a value of 128
```
http://192.168.235.110:8080/setgpio?gpio=2&state=pwm128
```

#### Batch Operations (Set GPIO 2 to HIGH and GPIO 4 to LOW)
```
http://192.168.235.110:8080/batch?operations=[{"gpio":2,"state":"high"},{"gpio":4,"state":"low"}]
```

#### Read GPIO 2 State
```
http://192.168.235.110:8080/readgpio?gpio=2
```

#### Schedule GPIO 2 to HIGH after 30 seconds
```
http://192.168.235.110:8080/schedule?gpio=2&state=high&delay=30000
```

#### Schedule GPIO 2 to HIGH after 30 seconds for 10 seconds
```
http://192.168.235.110:8080/schedule?gpio=2&state=high&delay=30000&duration=10000
```

#### Schedule GPIO 2 to LOW after 30 seconds
```
http://192.168.235.110:8080/schedule?gpio=2&state=low&delay=30000
```

#### Schedule GPIO 2 to PWM 128 after 30 seconds
```
http://192.168.235.110:8080/schedule?gpio=2&state=pwm128&delay=30000
```

These URLs cover setting GPIO states, batch operations, reading GPIO state, and scheduling operations with delays and optional durations. Adjust the GPIO pin numbers and states as needed for your testing purposes.
