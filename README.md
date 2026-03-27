
# VirtualGamePad Arduino Library

[![Arduino Library](https://img.shields.io/badge/Arduino-Library-blue.svg)](https://www.arduino.cc/reference/en/libraries/)
![License: GPL v3](https://img.shields.io/badge/License-GNU%20GENERAL%20PUBLIC%20LICENSE-blue.svg)


I was looking for some easy to use controller application that runs my Android mobile phone to control some __custom drones__ or __robot cars__. Unfortunately, I did not find anything that would meet the following requirements:

- At least one thumbstick (better two)
- Multiple buttons
- A defined communications protocol that can be used by Microcontrollers
- An Arduino Library to receive and process the data

I finally found the [VirtualGamepad](https://kitswas.github.io/VirtualGamePad/) open source project that fullfills all the criteria above, except the last.

This library fills the gap and enables your ESP32 microcontroller to act as a TCP server to receive the gamepad input data over WiFi, using the Colfer serialization format.

## Features

- Simple WiFi TCP server for ESP32
- Receives and decodes Colfer-encoded gamepad data
- Easy-to-use callback interface for handling gamepad state
- Supports all standard gamepad buttons and analog thumbsticks/triggers
- Compatible with the official VirtualGamePad client
- The ROS angles follow the right-hand rule and ROS conventions: 0° is forward (y+), +90° is left (x-), and -90° is right (x+), with counterclockwise as positive.

## Requirements

- ESP32 board or any other Microcontroller that supports the Arduino NetworkServer (e.g. WiFiServer)
- WiFi network 

## Installation

For Arduino, you can download the library as zip and call include Library -> zip library. Or you can git clone this project into the Arduino libraries folder e.g. with

```
cd  ~/Documents/Arduino/libraries
git clone https://github.com/pschatzmann/VirtualGamepadArduino.git
```



## Credits

- [VirtualGamePad](https://kitswas.github.io/VirtualGamePad/) project
- Colfer serialization format
