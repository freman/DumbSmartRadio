#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Preferences.h>
#include <RDA5807.h>
#include <WiFiManager.h>

// Instantiate the RDA5807 object
RDA5807 radio;

// Instantiate the mdns responder
MDNSResponder mdns;

// Instantiate the web server
ESP8266WebServer server(80);

// Setup Preferences
Preferences preferences;

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
	message += "RADIO BASS BOOST: " + String(radio.getBass()) + "\n";
	message += "RADIO ID: " + String(radio.getDeviceId()) + "\n\n";
	message +=
		"API Commands\n\
/set?args - Where ars can be any combination of the following seperated by &:\n\
\tvolume=10 - sets the volume to 10\n\
\tfrequency=8980 - sets the frequency to 89.8\n\
\tbass=1 - enables bass boost\n\
\thostname=radio - sets a new host/mdns name\n\
/save - store current values persistently\n\
/reset - performs a soft reset on the tuner\n";

	server.send(200, "text/plain", message);
}

// Function to handle the set endpoint
void handleSet() {
	String message = "";

	if (server.hasArg("volume")) {
		int volume = server.arg("volume").toInt();
		radio.setVolume(volume);
		message += "volume=" + String(radio.getVolume()) + "\n";
	}

	if (server.hasArg("frequency")) {
		int frequency = server.arg("frequency").toInt();
		radio.setFrequency(frequency);
		message += "frequency=" + String(radio.getFrequency()) + "\n";
	}

	if (server.hasArg("bass")) {
		bool bass = server.arg("bass") == "1";
		radio.setBass(bass);
		message += "bass=" + String(radio.getBass() ? "1" : "0") + "\n";
	}

	if (server.hasArg("hostname")) {
		String hostname = server.arg("hostname");
		if (mdns.setHostname(hostname)) {
      WiFi.setHostname(hostname.c_str()); 
			message += "hostname=" + hostname + "\n";
		} else {
			message += "hostname failed\n";
		}
	}

	if (message == "") {
		server.send(400, "text/plain", "No supported values passed");
		return;
	}

	server.send(200, "text/plain", message);
}

void handleReset() {
	radio.softReset();
	server.send(200, "text/plain", "Resetting");
}

// Function to handle save endpoint
void handleSave() {
	preferences.begin("dsr", false);

	preferences.putUChar("vol", radio.getVolume());
	preferences.putUShort("freq", radio.getFrequency());
	preferences.putBool("bass", radio.getBass());
  preferences.putString("hostname", WiFi.getHostname());

	preferences.end();

	server.send(200, "text/plain", "Values saved");
}

void setup() {
	// Start the serial communication
	Serial.begin(115200);
	Serial.println("");

	// Connect to Wi-Fi
	WiFiManager wifiManager;
	wifiManager.setConnectTimeout(30);
	wifiManager.autoConnect("DSR Configuration", "Bringing Music");

	// Init preferences namespace
	preferences.begin("dsr", true);

	// Load preferences
	uint8_t volume = preferences.getUChar("vol", 5);
	uint16_t frequency = preferences.getUShort("freq", 8980);
	bool bass = preferences.getBool("bass", true);
	String hostname = preferences.getString("hostname", "radio");

	preferences.end();

	radio.setup();
	radio.setVolume(volume);
	radio.setFrequency(frequency);
	radio.setBass(bass);

  WiFi.setHostname(hostname.c_str());
  mdns.begin(hostname);
	mdns.addService("http", "tcp", 80);

	// OTA setup
	ArduinoOTA.begin();

	// Set up server endpoints
	server.on("/", HTTP_GET, handleRoot);
	server.on("/set", HTTP_GET, handleSet);
	server.on("/save", HTTP_GET, handleSave);
	server.on("/reset", HTTP_GET, handleReset);

	// Start the server
	server.begin();

	Serial.println("HTTP server started");
}

void loop() {
	// Handle OTA updates
	ArduinoOTA.handle();

	// Handle HTTP requests
	server.handleClient();

  // Handle mdns
  mdns.update();
}