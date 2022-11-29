#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP_EEPROM.h>

// WIFI DEFINITIONS:
#define WiFiSSID "YOUR SSID"
#define WiFiPSK "WIFI PASSWORKD"

//MQTT DEFINITIONS:
#define mqtt_server "HOME ASSISTANT IP"
#define mqtt_clientID "ESP8266_WATER"
#define mqtt_user "MQTT USERNAME"
#define mqtt_password "MQTT PASSWORD"

// DEFINE VALUES:
#define DATA_PIN D7           // GPIO 13
#define PULSE_FACTOR 1        // pulses per liter
#define DEBOUCE_DELAY 500     // miliseconds debounce
#define SEND_FREQUENCY 10000  // miliseconds between MQTT publishing
#define START_VALUE 0         // 0 reads from EEPROM, value in liters will set EEPROM

// DEFINE VARIABLES:
volatile uint32_t pulseCount = 0;
uint32_t oldPulseCount = 0;
volatile uint32_t lastBlink = 0;
uint32_t newBlink = 0;
volatile double flow = 0;
double oldflow = 0;
uint32_t lastSend = 0;
uint32_t lastPulse = 0;
double volume = 0;

// START-UP SERVICES:
WiFiClient espClient;
PubSubClient client(espClient);

void connectWiFi()
{
  byte ledStatus = LOW;
//  WiFi.mode(WIFI_STA);                      // Set WiFi mode to station (as opposed to AP or AP_STA)
  WiFi.begin(WiFiSSID, WiFiPSK);            // Initiates a WiFI connection
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED)     // Use the WiFi.status() function to check if the ESP8266 is connected to a WiFi network.
  {  
    digitalWrite(BUILTIN_LED, ledStatus);   // Blink the LED
    Serial.print(".");
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH; 
    delay(100);                             // Delays allow the ESP8266 to setup wifi
  }
  Serial.println();
  Serial.print("WiFI Connected, IP address: ");
  Serial.println(WiFi.localIP());
} // end (connect to WiFi)

void connectMQTT() {
    Serial.print("Attempting MQTT server...");
    if (client.connect(mqtt_clientID, mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
    }
}

void eepromStart() {
  EEPROM.begin (sizeof(pulseCount));
  if (START_VALUE!=0 ) {
    EEPROM.put(0, START_VALUE * PULSE_FACTOR);
    boolean ok1 = EEPROM.commitReset();
    Serial.println((ok1) ? "Setting EEPROM OK" : "Setting EEPROM failed");
  }  
  EEPROM.get(0, pulseCount);
  Serial.print( "Read pulseCount from EEPROM: " );
  Serial.println(pulseCount);
}

void IRAM_ATTR pinTrigger();

// ------------------------ SETUP ------------------------
void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println();
  Serial.println("BOOTED...");
  pinMode(DATA_PIN, INPUT);
  connectWiFi();
  delay(100);
  client.setServer(mqtt_server, 1883);
  Serial.println("MQTT_server initiated on port 1883");
  delay(100);
  digitalWrite(BUILTIN_LED, HIGH);   // LED off
  eepromStart();
  delay(100);
  Serial.println("=== Ready to receive data ===");
  delay(500);
  attachInterrupt(digitalPinToInterrupt(DATA_PIN), pinTrigger, FALLING);
} 

// ------------------------ LOOP ------------------------
void loop() {
  if (WiFi.status() == 6) {
    connectWiFi();
  } 
  yield();
  PublishMQTT();
  yield();
}

void pinTrigger() {
  static unsigned long last_interrupt_time = 0;
  if ((millis() - last_interrupt_time) > DEBOUCE_DELAY)
  {
    Serial.println("Trigger");
    onPulse();
  }
  last_interrupt_time = millis();
}

void onPulse()
{
  uint32_t newBlink = micros();
  uint32_t interval = newBlink - lastBlink;
  if (interval != 0) {
    lastPulse = millis();
    if (interval < 500000L) {
      return;
    }
    flow = (60000000.0 / interval) / PULSE_FACTOR;
  }
  lastBlink = newBlink;
  pulseCount++;
}

void PublishMQTT() {
  if (!client.connected()) {
    connectMQTT();
  }
  if (client.connected()) {
    uint32_t currentTime = millis();
    if (currentTime - lastSend > SEND_FREQUENCY ) {
      lastSend = currentTime ;
      // Flow has changed
      if (flow != oldflow) {
        oldflow = flow;
        Serial.print("MQTT publish - flow (l/min): ");
        Serial.println(flow);
        client.publish("WATERMETER/WATER/FLOW", String(flow, 1).c_str());  
      }
      // No Pulse count received
      if (currentTime - lastPulse > SEND_FREQUENCY * 1.5) {
        flow = 0;
      }
      // Pulse count has changed
      if (pulseCount != oldPulseCount) {
        oldPulseCount = pulseCount;
        volume = ((double)pulseCount / (double)PULSE_FACTOR / 1000);
        Serial.print("MQTT publish - volume (m3): ");
        Serial.println(volume, 3);
        client.publish("WATERMETER/WATER/VOLUME", String(volume, 3).c_str());  
        EEPROM.put(0, pulseCount);
        boolean ok1 = EEPROM.commitReset();
        if (ok1) {
          Serial.print("EEPROM put pulseCount: ");
          Serial.println(pulseCount);
          } else {
          Serial.println("EEPRM put failed");
        }       
      }
      client.loop();
    }
  }
}
