#include <Wire.h>
#include <RDA5807.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <DNSServer.h>
#include <WiFiManager.h>

// Instantiate the RDA5807 object
RDA5807 radio;

// Instantiate the web server
ESP8266WebServer server(80);

// EEPROM addresses for storing volume and frequency
int eepromVolumeAddress = 0;
int eepromFrequencyAddress = sizeof(int);
int eepromBassAddress = sizeof(int) * 2;


// Function to handle root endpoint
void handleRoot() {
  String message = "DSR Status\n\n";
  message += "Free Heap: " + String(ESP.getFreeHeap()) + "\n";
  message += "WIFI SSID: " + WiFi.SSID() + "\n";
  message += "WIFI BSSID: " + WiFi.BSSIDstr() + "\n";
  message += "WIFI RSSI: " + String(WiFi.RSSI()) + "\n";
  message += "RADIO Frequency: " + String(radio.getFrequency()) + "\n";
  message += "RADIO Volume: " + String(radio.getVolume()) + "\n";
  message += "RADIO RSSI: " + String(radio.getRssi()) + "\n";
  message += "RADIO STEREO: " + String(radio.isStereo()) + "\n";
  message += "RADIO BASS BOOST: " + String(radio.getBass()) + "\n\n";
  message += "API Commands\n\n/setVolume?volume=10 - Set the volume to 10\n/setFrequency?frequency=8980 - Set the frequency to 89.8\n/setBass?bass=1 - Turn on bass boost\n/save - Save values to EEPROM";

  server.send(200, "text/plain", message);
}

// Function to handle set volume endpoint
void handleSetVolume() {
  if (server.hasArg("volume")) {
    int volume = server.arg("volume").toInt();
    radio.setVolume(volume);
    server.send(200, "text/plain", "Volume set to " + String(radio.getVolume()));

    return;
  }

  server.send(400, "text/plain", "Argument 'volume' missing");
}

// Function to handle set frequency endpoint
void handleSetFrequency() {
  if (server.hasArg("frequency")) {
    int frequency = server.arg("frequency").toInt();
    radio.setFrequency(frequency);
    server.send(200, "text/plain", "Frequency set to " + String(radio.getFrequency()));
  }

  server.send(400, "text/plain", "Argument 'frequency' missing");
}

// Function to handle set bass endpoint
void handleSetBass() {
  if (server.hasArg("bass")) {
    bool bass = server.arg("bass") == "1";
    radio.setBass(bass);
    server.send(200, "text/plain", "Bass set to " + String(radio.getBass()));
  }

  server.send(400, "text/plain", "Argument 'bass' missing");
}

// Function to handle save endpoint
void handleSave() {
  EEPROM.put(eepromVolumeAddress, radio.getVolume());
  EEPROM.put(eepromFrequencyAddress, radio.getFrequency());
  EEPROM.put(eepromBassAddress, radio.getBass());

  EEPROM.commit();
  server.send(200, "text/plain", "Values saved to EEPROM");
}

void setup() {
  // Start the serial communication
  Serial.begin(115200);
  Serial.println("");

  // Connect to Wi-Fi
  WiFiManager wifiManager;
  wifiManager.autoConnect("DSR Configuration", "Bringing Music");

  int volume = 5; // Initial volume
  int frequency = 8980; // Initial frequency
  bool bass = true; // Initial bass state

  // Read values from EEPROM
  EEPROM.get(eepromVolumeAddress, volume);
  EEPROM.get(eepromFrequencyAddress, frequency);
  EEPROM.get(eepromBassAddress, bass);

  radio.setup();

  radio.setVolume(volume);
  radio.setFrequency(frequency);
  radio.setBass(bass);

  // OTA setup
  ArduinoOTA.begin();
  
  // Set up server endpoints
  server.on("/", HTTP_GET, handleRoot);
  server.on("/setVolume", HTTP_GET, handleSetVolume);
  server.on("/setFrequency", HTTP_GET, handleSetFrequency);
  server.on("/setBass", HTTP_GET, handleSetBass);
  server.on("/save", HTTP_GET, handleSave);

  // Start the server
  server.begin();

  Serial.println("HTTP server started");
}

void loop() {
  // Handle OTA updates
  ArduinoOTA.handle();

  // Handle HTTP requests
  server.handleClient();
}