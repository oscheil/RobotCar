/** @file doorsensor.ino 
 */
// markup for doxygen.org

#define DEBUG 1  ///< Define DEBUG to get the Output from DEBUG_PRINTLN
// 2018-03-11, Rudolf Reuter changes: 
//             Analog calibration with dresistor voltage divider, 
//             1.65 -> 0.25 mA battery current consumption in deep sleep mode,
//             avoid sleep while in configuration setup,
//             print wakeup reason for debug reason.
//             Battery LOW at 3.4 V
//Include Basecamp in this sketch
#include <Basecamp.hpp>
#include <esp_wifi.h>  // for WiFi power dowm

//Create a new Basecamp instance called iot
Basecamp iot;

//Variables for the sensor and the battery
gpio_num_t SensorPin = GPIO_NUM_33;
static const int BatteryPin = 34;  // =A6
static const int BatteryPinA = A6; // use divider 
int sensorValue = 0;
int level;          // sensor level for wakeup

float temperature;  // in °C

int VoltBat = 0;    //  ADC value of the battery
// measure once the battery voltage and read the ADC value VoltBat, then update VoltCal.
// 4.13 V reads VoltBat 2870 / 4.13 = 695 for Volt calculation + calibration
int VoltCal = 695;  // apply the calculated value
//  Voltage scaler, expand voltage range, eliminate 0.1 V offset, resistors 5% or better 
//          3.3 V --+3M3+--+--- ADC pin 34, 0.1 V...2.2 V, ADC -6dB
//                         |
// VBat 0 - 6.6 V --+220K+-+--+100K+---| GND
//
// 3.3 V ---+reed-contact+---Pin 33---+120K---| GND, pull down resistor

//The batteryLimit defines the point at which the battery is considered empty.
int batteryLimit = 2363;  // = 3.4 V * VoltCal

//This is used to control if the ESP should enter sleep mode or not
bool delaySleep = false;
esp_err_t esp_err;

//Variables for the mqtt packages and topics
uint16_t statusPacketIdSub = 0;
String delaySleepTopic;
String statusTopic;
String batteryTopic;
String batteryValueTopic;


/**
  *@brief Method to print the reason by which ESP32.
  *       has been awaken from sleep
 */
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case 1  : DEBUG_PRINTLN("Wakeup caused by external signal using RTC_IO"); break;
    case 2  : DEBUG_PRINTLN("Wakeup caused by external signal using RTC_CNTL"); break;
    case 3  : DEBUG_PRINTLN("Wakeup caused by timer"); break;
    case 4  : DEBUG_PRINTLN("Wakeup caused by touchpad"); break;
    case 5  : DEBUG_PRINTLN("Wakeup caused by ULP program"); break;
    default : DEBUG_PRINTLN("Wakeup was not caused by deep sleep"); break;
  }
}


/**
 * @brief  Setup doorsensor and basecamp.
 * @param  use Serial() with 115200 Baud 8N1
 * @return Nothing
 */
void setup() {
  int i = 0;
  //configuration of the battery and sensor pins
  //pinMode(SensorPin, INPUT_PULLDOWN); // comment if 120 KOhm resistor is used
  pinMode(SensorPin, INPUT); // if 120 KOhm resistor for pull down is provided
  pinMode(BatteryPin, INPUT);
  analogSetPinAttenuation(BatteryPinA, ADC_6db); // for better linearity

  // read the status of the doorsensor as soon as possible to determine 
  // the state that triggered it.
  sensorValue = digitalRead(SensorPin);



  //Initialize Basecamp
  iot.begin();

  //temperature = temperatureRead();  // Test: 21°C shows 59.4 °C
  //DEBUG_PRINT("Temperature °C: ");
  //DEBUG_PRINTLN(temperature);

  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  VoltBat = analogRead(BatteryPin); // first value deviates too much!
  //for (int n2 = 0; n2 < 16; n2++) { // Debug: for checking ADC stability
  for (int n = 0; n < 16; n++) {     // average
    VoltBat = analogRead(BatteryPin);
    //DEBUG_PRINTLN(VoltBat);  // for debug
    i += VoltBat;
  }
  VoltBat = i / 16;
  DEBUG_PRINT("ADC Average (n=16): ");
  DEBUG_PRINTLN(VoltBat);
  //delay(1000);
  //i = 0;
  //}  // end of Debug

  //Configure the MQTT topics
  delaySleepTopic = "cmd/" + iot.hostname + "/delaysleep";
  statusTopic = "stat/" + iot.hostname + "/status";
  batteryTopic = "stat/" + iot.hostname + "/battery";
  batteryValueTopic = "stat/" + iot.hostname + "/batteryvalue";

  //Set up the Callbacks for the MQTT instance. Refer to the Async MQTT Client documentation
  iot.mqtt.onConnect(onMqttConnect);
  iot.mqtt.onPublish(suspendESP);
  iot.mqtt.onMessage(onMqttMessage);
} // setup()


/**
 * @brief This function is called when the MQTT-Server is connected.
 * @param sessionPresent: session present
 * @return Nothing
 */
void onMqttConnect(bool sessionPresent) {
  //Subscribe to the delay topic
  iot.mqtt.subscribe(delaySleepTopic.c_str(), 0);
  //Trigger the transmission of the current state.
  transmitStatus();
}


/**
 * @brief This function transfers the state of the sensor.
          That includes the door status, battery status and level
 * @return Nothing
 */
void transmitStatus() {
  if (sensorValue == 0) {
    level = 1;  // remember opposite level for wakeup
    DEBUG_PRINT("Door open: ");
    DEBUG_PRINTLN(sensorValue);
    //Transfer the current state of the sensor to the MQTT broker
    statusPacketIdSub = iot.mqtt.publish(statusTopic.c_str(), 1, true, "open" );
  } else {
    level = 0;  // remember opposite level for wakeup
    DEBUG_PRINT("Door closed: ");
    DEBUG_PRINTLN(sensorValue);
    //Transfer the current state of the sensor to the MQTT broker
    statusPacketIdSub = iot.mqtt.publish(statusTopic.c_str(), 1, true, "closed" );
  }

  //Read the current analog battery value
  sensorValue = analogRead(BatteryPin);
  VoltBat = sensorValue * 100 / VoltCal;  // convert to Volt * 100
  //sensorC stores the battery value as a char
  char sensorC[6];
  //convert the sensor value to a string
  sprintf(sensorC, "%03i", VoltBat);
  //Send the sensor value to the MQTT broker
  iot.mqtt.publish(batteryValueTopic.c_str(), 1, true, sensorC);
  //Check the battery level and publish the state
  if (sensorValue < batteryLimit) {
    DEBUG_PRINTLN("Battery empty");
    iot.mqtt.publish(batteryTopic.c_str(), 1, true, "empty" );
  } else {
    DEBUG_PRINTLN("Battery full");
    iot.mqtt.publish(batteryTopic.c_str(), 1, true, "full" );
  }
  DEBUG_PRINTLN("Data published");
} // transmitStatus()


/**
 * @brief  This topic is called if an MQTT message is received.
 * @param  topic: topic for MQTT
 * @param  payload: ???
 * @param  AsyncMqttClientMessageProperties properties: MQTT properties
 * @param  len: ???
 * @param  index: ???
 * @param  total: ???
 * @return Nothing
 */
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  //Check if the payload eqals "true" and set delaySleep correspondigly
  //Since we only subscribed to one topic, we only have to compare the payload
  if (strcmp(payload, "true") == 0)  {
    delaySleep = true;
  } else  {
    delaySleep = false;
  }
}


/**
 * @brief This function is called when the MQTT-Server is connected.
 * @param packetId: MQTT packet ID
 * @return Nothing
 */
void suspendESP(uint16_t packetId) {
  //Check if the published package is the one of the door sensor
  if (packetId == statusPacketIdSub) {
   
    if (delaySleep == true) {
      DEBUG_PRINTLN("Delaying Sleep");
      return;
    }
    DEBUG_PRINTLN("Entering deep sleep");
    //properly disconnect from the MQTT broker
    iot.mqtt.disconnect();
    //esp_wifi_deinit();  // not needed
    esp_wifi_stop();  // 0.20 mA sleep current (battery)
    // Note: ext1 needs an external pull down resistor
    esp_sleep_enable_ext0_wakeup((gpio_num_t)SensorPin, level); 
    //send the ESP into deep sleep
    esp_deep_sleep_start();        // 1.63 mA, with esp_wifi_stop() 0.20 mA
  }
}


/**
 * @brief Arduino main loop.
 * @return Nothing
 */
void loop() {
  // if not configured, wait
  if (iot.configuration.get("WifiConfigured") != "True") {
    DEBUG_PRINTLN("WiFiConfig NOT configured ");
    delay(2000);
  } else {
    // /!\ This time is too short for parameter setup in hotspot mode
    // Check if the ESP is up for more than 30s and shut it down to save the battery.
    delay(2000); // wait for RESET, if just configured
    if (millis() > 30000) {
       DEBUG_PRINTLN("Entering deep sleep after 30s of idle");
       esp_deep_sleep_start();
    }
  }
  delay(10);
}
