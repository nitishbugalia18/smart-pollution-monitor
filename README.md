# IoT Smart Pollution & Safety Monitoring System

## About
An IoT-based system that monitors air quality, temperature, humidity, and alcohol levels.
It automatically activates a ventilation fan, sounds a buzzer, and locks ignition if unsafe conditions are detected.

## Hardware Used
- ESP8266 NodeMCU
- MQ-135 (Gas/Air Quality Sensor)
- MQ-3 (Alcohol Sensor)
- DHT11 (Temperature & Humidity Sensor)
- Relay Module (Fan control)
- Buzzer

## How It Works
1. Sensors continuously read environmental data
2. ESP8266 compares values against thresholds
3. If unsafe: fan turns on, buzzer sounds, ignition locks
4. Data is sent to Flask server via WiFi
5. Mobile app receives real-time alerts
