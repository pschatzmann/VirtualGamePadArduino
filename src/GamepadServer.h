#pragma once

#include <errno.h>

#include <vector>

#include "NetworkServer.h"

extern "C" {
#include "VGP_Data_Exchange/C/Colfer.h"
#include "VGP_Data_Exchange/C/GameButtons.h"
}

/**
 * @brief Represents the state of a gamepad thumbstick, including position and
 * angle helpers.
 *
 * Provides both Cartesian (x, y) and polar (angle, magnitude) representations.
 * Includes helpers for ROS and standard angle conventions.
 *
 * - x: horizontal axis, -1.0 (left) to 1.0 (right)
 * - y: vertical axis, -1.0 (down) to 1.0 (up)
 * - angle(): radians, 0 = x-axis (forward)
 * - angleROS(): radians, ROS convention (0 = forward/x, 90 = left/y, -90 =
 * right/-y)
 * - angleDeg(): degrees, 0 = x-axis (forward)
 * - angleDegROS(): degrees, ROS convention (0 = forward/x, 90 = left/y, -90 =
 * right/-y)
 * - magnitude(): stick displacement, 0.0 (center) to 1.0 (edge)
 */
struct ThumbstickResult {
  float x = 0.0f;
  float y = 0.0f;
  ThumbstickResult() = default;
  ThumbstickResult(float x, float y) : x(x), y(y) {}
  float angle() const { return atan2(y, x); }  // radians, 0 = x-axis (forward)
  float angleROS() const {
    if (x == 0.0f && y == 0.0f)
      return 0.0f;  // Handle center position as 0 degrees
    float result = atan2(x, y) + M_PI;
    return result > M_PI ? result - 2 * M_PI : result;
  }  // radians, ROS: 0 = forward (y), 90 = left (x-), -90 = right (x+)
  float angleDeg() const {
    return angle() * 180.0f / M_PI;
  }  // degrees, 0 = x-axis (forward)
  float angleDegROS() const {
    return angleROS() * 180.0f / M_PI;
  }  // degrees, ROS: 0 = forward (y), 90 = left (x-), -90 = right (x+)
  // magnitude 0.0 to 1.0
  float magnitude() const { return sqrt(x * x + y * y); }
};

/**
 * @brief Represents the full state of a gamepad, including all button states
 * and thumbsticks.
 */
struct GamepadState {
  // Button states
  bool a, b, x, y;
  bool menu, view;
  bool dpad_up, dpad_down, dpad_left, dpad_right;
  bool left_shoulder, right_shoulder;
  bool left_thumbstick_button, right_thumbstick_button;
  float left_trigger, right_trigger;
  // Thumbsticks
  ThumbstickResult left_thumbstick;
  ThumbstickResult right_thumbstick;

  GamepadState()
      : a(false),
        b(false),
        x(false),
        y(false),
        menu(false),
        view(false),
        dpad_up(false),
        dpad_down(false),
        dpad_left(false),
        dpad_right(false),
        left_shoulder(false),
        right_shoulder(false),
        left_thumbstick_button(false),
        right_thumbstick_button(false),
        left_trigger(0.0f),
        right_trigger(0.0f),
        left_thumbstick(),
        right_thumbstick() {}
  GamepadState(const vgp_data_exchange_gamepad_reading& reading)
      : a(reading.buttons_down & GamepadButtons_A),
        b(reading.buttons_down & GamepadButtons_B),
        x(reading.buttons_down & GamepadButtons_X),
        y(reading.buttons_down & GamepadButtons_Y),
        menu(reading.buttons_down & GamepadButtons_Menu),
        view(reading.buttons_down & GamepadButtons_View),
        dpad_up(reading.buttons_down & GamepadButtons_DPadUp),
        dpad_down(reading.buttons_down & GamepadButtons_DPadDown),
        dpad_left(reading.buttons_down & GamepadButtons_DPadLeft),
        dpad_right(reading.buttons_down & GamepadButtons_DPadRight),
        left_shoulder(reading.buttons_down & GamepadButtons_LeftShoulder),
        right_shoulder(reading.buttons_down & GamepadButtons_RightShoulder),
        left_thumbstick_button(reading.buttons_down &
                               GamepadButtons_LeftThumbstick),
        right_thumbstick_button(reading.buttons_down &
                                GamepadButtons_RightThumbstick),
        left_trigger(reading.left_trigger),
        right_trigger(reading.right_trigger),
        left_thumbstick(reading.left_thumbstick_x, reading.left_thumbstick_y),
        right_thumbstick(reading.right_thumbstick_x,
                         reading.right_thumbstick_y) {}
  // No default constructor: GamepadState() = default; is not valid due to
  // ThumbstickResult
};

/// Callback type for when a new gamepad state is received
typedef void (*GamepadStateCallback)(const GamepadState&, void* ref);

/**
 * @class GamepadServer
 * @brief Simple TCP server for ESP32 to receive Colfer-encoded gamepad data.
 *
 * This class sets up a WiFi TCP server on the ESP32, receives binary data from
 * a client, and attempts to decode it as Colfer-encoded
 * vgp_data_exchange_gamepad_reading messages. It uses a buffer to handle
 * partial/incomplete messages robustly, similar to the official PC server.
 *
 * Usage:
 *   1. Instantiate with your WiFi credentials and (optionally) port.
 *   2. Call begin() in setup().
 *   3. Call handleClient() in loop().
 *   4. Register a callback with setCallback() to process received gamepad
 * readings.
 *
 * Example:
 *   GamepadServer server("SSID", "password");
 *   void setup() {
 *     Serial.begin(115200);
 *     server.begin();
 *     server.setCallback([](const GamepadState& state) {
 *       // Example: check if A button is pressed
 *       if (state.a) {
 *         Serial.println("A button pressed");
 *       }
 *       if (state.b) {
 *         Serial.println("B button pressed");
 *       }
 *       Serial.print("Left stick angle: ");
 *       Serial.println(state.left_thumbstick.angle());
 *       Serial.print("Left stick magnitude: ");
 *       Serial.println(state.left_thumbstick.magnitude());
 *     });
 *   }
 *   void loop() {
 *     server.handleClient();
 *   }
 *
 * The protocol expects raw Colfer-encoded vgp_data_exchange_gamepad_reading
 * messages, as sent by the official VirtualGamePad client/server.
 */

class GamepadServer {
 public:
  /// Default constructor: takes a reference to a NetworkServer (e.g.
  /// WiFiServer)
  GamepadServer(NetworkServer& server) : _server(&server) {}

  /// Start the server. Call in setup() after instantiating GamepadServer.
  void begin() { _server->begin(); }

  /// Set the callback to be called when a gamepad reading is received
  void setCallback(GamepadStateCallback cb, void* ref = this) {
    _callback = cb;
    _callbackRef = ref;
  }

  /// To be called in loop to handle incoming client connections and data.
  void handleClient() {
    static std::vector<uint8_t> buffer;
    auto client = _server->available();
    if (client) {
      while (client.connected()) {
        while (client.available()) {
          uint8_t byte = client.read();
          buffer.push_back(byte);
        }
        processBuffer(buffer);
        delay(10);
      }
      client.stop();
    }
  }

  /// Provide the actual state
  GamepadState& getState() { return _state; }

 private:
  NetworkServer* _server = nullptr;
  GamepadStateCallback _callback = nullptr;
  GamepadState _state;
  void* _callbackRef = this;

  // Helper: process as many complete packets as possible from the buffer
  void processBuffer(std::vector<uint8_t>& buffer) {
    while (!buffer.empty()) {
      vgp_data_exchange_gamepad_reading reading;
      memset(&reading, 0, sizeof(reading));
      size_t used = vgp_data_exchange_gamepad_reading_unmarshal(
          &reading, buffer.data(), buffer.size());
      if (used == 0) {
        if (errno == EWOULDBLOCK) {
          // Incomplete data, wait for more
          break;
        } else if (errno == EFBIG || errno == EILSEQ) {
          // Data too large or schema mismatch, drop first byte and try again
          buffer.erase(buffer.begin());
          continue;
        } else {
          // Unknown error, clear buffer
          buffer.clear();
          break;
        }
      } else {
        // Success
        _state = GamepadState(reading);
        if (_callback) _callback(_state, callbackRef);
        buffer.erase(buffer.begin(), buffer.begin() + used);
      }
    }
  }
};
