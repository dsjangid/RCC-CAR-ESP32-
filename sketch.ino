#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// --- WiFi Configuration ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// --- MQTT Broker Configuration ---
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* client_id = "ESP32_RCCar_MeyDivyansh"; // Ensure this matches dashboard sub-topic
const char* control_topic = "rccar/meydivyansh/control";
const char* telemetry_topic = "rccar/meydivyansh/telemetry";

// --- Hardware Pin Definitions ---
// Left Motor (L293D Inputs & Enable)
const int pinENA = 12; // Speed PWM Left
const int pinIN1 = 13; // Direction Input 1 Left
const int pinIN2 = 14; // Direction Input 2 Left

// Right Motor (L293D Inputs & Enable)
const int pinENB = 27; // Speed PWM Right
const int pinIN3 = 26; // Direction Input 3 Right
const int pinIN4 = 25; // Direction Input 4 Right

// HC-SR04 Ultrasonic Distance Sensor
const int pinTrig = 33;
const int pinEcho = 32;

// --- System Variables ---
WiFiClient espClient;
PubSubClient client(espClient);
long lastTelemetryTime = 0;
const int telemetryInterval = 500; // Send telemetry every 500ms
long lastPingTimestamp = 0;        // Echo back for latency calculation

// Movement state variables
char currentDir = 'S';
int currentSpeed = 0; 
bool autoStopped = false;

// Function declarations
void setupWiFi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnectMQTT();
void driveCar(char direction, int speed);
float getDistance();
void sendTelemetry();

void setup() {
  Serial.begin(115200);
  Serial.println("--- Booting APEX RC CAR System ---");

  // Initialize motor control pins
  pinMode(pinENA, OUTPUT);
  pinMode(pinIN1, OUTPUT);
  pinMode(pinIN2, OUTPUT);
  pinMode(pinENB, OUTPUT);
  pinMode(pinIN3, OUTPUT);
  pinMode(pinIN4, OUTPUT);

  // Initialize sensor pins
  pinMode(pinTrig, OUTPUT);
  pinMode(pinEcho, INPUT);

  // Ensure motors are initially stopped
  driveCar('S', 0);

  // Connect to WiFi
  setupWiFi();

  // Configure MQTT client
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  // Collision Avoidance logic (Auto Stop if object is too close)
  float distance = getDistance();
  if (distance > 0 && distance <= 15.0 && currentDir == 'F') {
    if (!autoStopped) {
      Serial.printf("[AUTO-STOP] Obstacle detected at %.2f cm! Emergency Braking.\n", distance);
      driveCar('S', 0);
      autoStopped = true;
      sendTelemetry(); // Trigger immediate telemetry update
    }
  } else if (autoStopped && distance > 15.0) {
    // Reset auto-stop condition when obstacle is cleared
    autoStopped = false;
  }

  // Periodic Telemetry Sending
  long now = millis();
  if (now - lastTelemetryTime > telemetryInterval) {
    lastTelemetryTime = now;
    sendTelemetry();
  }
}

void setupWiFi() {
  delay(10);
  Serial.printf("\nConnecting to SSID: %s", ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// MQTT callback to process incoming messages
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.printf("\nMessage arrived on topic: [%s]\n", topic);
  
  if (strcmp(topic, control_topic) == 0) {
    // Allocate space for JSON parsing
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payload, length);

    if (error) {
      Serial.print("JSON deserialization failed: ");
      Serial.println(error.f_str());
      return;
    }

    // Extract values
    const char* dirStr = doc["dir"];
    int speedVal = doc["speed"];
    long pingVal = doc["ping"];

    if (dirStr) {
      char direction = dirStr[0];
      lastPingTimestamp = pingVal;
      
      Serial.printf("Command - Dir: %c, Speed PWM: %d, Latency Ref: %ld\n", direction, speedVal, pingVal);

      // Block forward commands if obstacle is too close
      if (direction == 'F' && getDistance() <= 15.0) {
        Serial.println("[BLOCKED] Obstacle ahead. Cannot move forward.");
        driveCar('S', 0);
        autoStopped = true;
      } else {
        driveCar(direction, speedVal);
      }
    }
  }
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Generate a random Client ID to prevent conflicts on public broker
    String dynamicClientId = String(client_id) + "-" + String(random(0xffff), HEX);
    
    if (client.connect(dynamicClientId.c_str())) {
      Serial.println("CONNECTED to broker!");
      client.subscribe(control_topic);
      Serial.printf("Subscribed to control topic: %s\n", control_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" - Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

// Controls the L293D pins based on target direction and PWM speed
void driveCar(char direction, int speed) {
  currentDir = direction;
  currentSpeed = speed;

  switch (direction) {
    case 'F': // Forward
      digitalWrite(pinIN1, HIGH);
      digitalWrite(pinIN2, LOW);
      digitalWrite(pinIN3, HIGH);
      digitalWrite(pinIN4, LOW);
      analogWrite(pinENA, speed);
      analogWrite(pinENB, speed);
      break;

    case 'B': // Backward
      digitalWrite(pinIN1, LOW);
      digitalWrite(pinIN2, HIGH);
      digitalWrite(pinIN3, LOW);
      digitalWrite(pinIN4, HIGH);
      analogWrite(pinENA, speed);
      analogWrite(pinENB, speed);
      break;

    case 'L': // Turn Left (Spin on spot or sharp turn)
      digitalWrite(pinIN1, LOW);
      digitalWrite(pinIN2, HIGH);
      digitalWrite(pinIN3, HIGH);
      digitalWrite(pinIN4, LOW);
      analogWrite(pinENA, speed);
      analogWrite(pinENB, speed);
      break;

    case 'R': // Turn Right (Spin on spot or sharp turn)
      digitalWrite(pinIN1, HIGH);
      digitalWrite(pinIN2, LOW);
      digitalWrite(pinIN3, LOW);
      digitalWrite(pinIN4, HIGH);
      analogWrite(pinENA, speed);
      analogWrite(pinENB, speed);
      break;

    case 'S': // Stop
    default:
      digitalWrite(pinIN1, LOW);
      digitalWrite(pinIN2, LOW);
      digitalWrite(pinIN3, LOW);
      digitalWrite(pinIN4, LOW);
      analogWrite(pinENA, 0);
      analogWrite(pinENB, 0);
      break;
  }
}

// Reads distance using the HC-SR04 ultrasonic sensor
float getDistance() {
  digitalWrite(pinTrig, LOW);
  delayMicroseconds(2);
  
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrig, LOW);

  long duration = pulseIn(pinEcho, HIGH, 30000); // 30ms timeout (~5m max)
  
  if (duration == 0) {
    return 400.0; // Return max distance if no pulse received
  }
  
  float distanceCm = duration * 0.0343 / 2.0;
  return distanceCm;
}

// Sends JSON telemetry data to the broker
void sendTelemetry() {
  StaticJsonDocument<256> teleDoc;
  
  teleDoc["distance"] = getDistance();
  teleDoc["uptime"] = millis() / 1000;
  teleDoc["ping"] = lastPingTimestamp; // Return ping timestamp for latency calculations
  
  char buffer[256];
  serializeJson(teleDoc, buffer);
  
  client.publish(telemetry_topic, buffer);
  // Optional: Serial print for local simulation debugging
  // Serial.print("Telemetry Sent: ");
  // Serial.println(buffer);
}
