/* DS mailbox automation
 * * Sketch wrapper. See MySystem.h for selection of which module to compile
 * (c) DNS 2020-2023
 * Tested with:
 * * Software:
 * * * Arduino 1.8.19 (https://www.arduino.cc/en/software)
 * * * ESP8266 Core for Arduino 3.0.2 (https://github.com/esp8266/Arduino)
 * * * ESP8266 LittleFS plugin 2.6.0 (used to upload the icon file to file system) https://github.com/earlephilhower/arduino-esp8266littlefs-plugin
 * * * ESP-DS-System 1.1.3 (https://github.com/denis-stepanov/esp-ds-system)
 * * * WiFiManager 0.16.0 (https://github.com/tzapu/WiFiManager)
 * * * JLED 4.12.0 (https://github.com/jandelgado/jled)
 * * * AceButton 1.9.2 (https://github.com/bxparks/AceButton)
 * * * ArduinoJson 6.20.0 (https://github.com/bblanchon/ArduinoJson)
 * * * UniversalTelegramBot 1.3.0 (https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot)
 * * * Uptime-Library 1.0.0 (https://github.com/YiannisBourkelis/Uptime-Library)
 * * Hardware:
 * * * Local module:
 * * * * Prod:
 * * * * * ESP-01 1MB
 * * * * * Arduino IDE settings:
 * * * * * * Board: Generic ESP8266 Module
 * * * * * * Upload Speed: 115200
 * * * * * * Flash Size: 1MB (FS:256kB)
 * * * * * * Flash Mode: DOUT (compatible) (depends on the module - yours could be different)
 * * * * * * Builtin LED: 1
 * * * * Dev: (to enable, define DS_DEVBOARD in MySystem.h)
 * * * * * ESP-12F Witty Cloud. Incoming message emulated with button double click
 * * * * * Arduino IDE settings:
 * * * * * * (same as for prod, except)
 * * * * * * Upload Speed: 460800
 * * * * * * Builtin LED: 2
 * * * Remote module:
 * * * * Prod:
 * * * * * ESP-01S 1MB. Arduino IDE settings:
 * * * * * * Board: Generic ESP8266 Module
 * * * * * * Flash Size: 1MB (FS:none)
 * * * * * * Flash Mode: QIO (fast) (depends on the module - yours could be different)
 */

#define DS_APP_NAME    "ESP8266 DS Mailbox Automation"
#define DS_APP_VERSION "2.2b5"
#define DS_APP_URL     "https://github.com/denis-stepanov/esp8266-mailbox"

#include "local.ino.h"        // Local module
#include "remote.ino.h"       // Remote module
