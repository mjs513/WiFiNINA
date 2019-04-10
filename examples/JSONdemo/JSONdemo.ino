/*
This example creates a client object that connects and transfers
data using always SSL, then shows how to parse a JSON document in an HTTP response.

It is compatible with the methods normally related to plain
connections, like client.connect(host, port).

Written by Arturo Guadalupi + Copyright Benoit Blanchon 2014-2019
last revision November 2015

*/

#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>

// Configure the pins used for the ESP32 connection
#if !defined(SPIWIFI_SS)  // if the wifi definition isnt in the board variant
  // Don't change the names of these #define's! they match the variant ones
  #define SPIWIFI     SPI
  #define SPIWIFI_SS    10  // Chip select pin
  #define SPIWIFI_ACK   7   // a.k.a BUSY or READY pin
  #define ESP32_RESETN  5   // Reset pin
  #define ESP32_GPIO0   -1  // Not connected
#endif

#include "arduino_secrets.h" 
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)

#define SERVER "cdn.syndication.twimg.com"
#define PATH   "/widgets/followbutton/info.json?screen_names=adafruit"

// Initialize the SSL client library
// with the IP address and port of the server
// that you want to connect to (port 443 is default for HTTPS):
WiFiSSLClient client;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the WiFi module:
  WiFi.setPins(SPIWIFI_SS, SPIWIFI_ACK, ESP32_RESETN, ESP32_GPIO0, &SPIWIFI);
  while (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    delay(1000);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  do {
    status = WiFi.begin(ssid, pass);
    delay(100); // wait until connected
  } while (status != WL_CONNECTED);
  Serial.println("Connected to wifi");
  printWifiStatus();

}

uint32_t bytes = 0;

void loop() {
  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (client.connect(SERVER, 443)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    client.println("GET " PATH " HTTP/1.1");
    client.println("Host: " SERVER);
    client.println("Connection: close");
    client.println();
  }

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return;
  }

  // wait until we get a double blank line
  client.find("\r\n\r\n", 4);

  // Allocate the JSON document
  // Use arduinojson.org/v6/assistant to compute the capacity.
  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(8) + 200;
  DynamicJsonDocument doc(capacity);

  // Parse JSON object
  DeserializationError error = deserializeJson(doc, client);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  // Extract values
  JsonObject root_0 = doc[0];
  Serial.println(F("Response:"));
  const char* root_0_screen_name = root_0["screen_name"];
  long root_0_followers_count = root_0["followers_count"];

  Serial.print("Twitter username: "); Serial.println(root_0_screen_name);
  Serial.print("Twitter followers: "); Serial.println(root_0_followers_count);
  
  // Disconnect
  client.stop();

  delay(10000);
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
