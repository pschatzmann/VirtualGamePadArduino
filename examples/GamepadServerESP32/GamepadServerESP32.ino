// Example usage of GamepadServer on ESP32. Connects to WiFi, starts a TCP
// server on port 8080, and prints received gamepad states to the Serial
// console.
#include "GamepadServer.h"
#include "WiFi.h"

const char* ssid = "your-ssid";
const char* password = "your-password";
WiFiServer wifiServer(8080);
GamepadServer server(wifiServer);

void setup() {
  Serial.begin(115200);
  // setup Wifi
  WiFi.setSleep(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  // Setup server: using lambda callback to print some received gamepad states
  server.setCallback([](const GamepadState& state, void*) {
    if (state.a) Serial.println("A button pressed");
    if (state.b) Serial.println("B button pressed");
    if (state.x) Serial.println("X button pressed");
    if (state.y) Serial.println("Y button pressed");

    Serial.print("Left stick angle: ");
    Serial.println(state.left_thumbstick.angleDegROS());
    Serial.print("Left stick magnitude: ");
    Serial.println(state.left_thumbstick.magnitude());
  });
  server.begin();
}

void loop() { server.handleClient(); }