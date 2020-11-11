/* DS mailbox automation
 * * ESP8266 Wi-Fi disabling sketch for remote module
 * (c) DNS 2020
 * * This program is optional. It helps reducing battery power consumption of the remote module on cold start (rare event)
 * * On warm start (normal mode) the remote module wakes up with the RF module switched off, so Wi-Fi settings do not matter
 * * The setting is persistent, so this sketch needs to be loaded once before uploading the actual remote module program
 */

#include <ESP8266WiFi.h>

void setup() {

  WiFi.mode(WIFI_OFF);
  
}

void loop() {

}
