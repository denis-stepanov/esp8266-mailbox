/* DS mailbox automation
 * * Remote module
 * (c) DNS 2020
 */

#include "MySystem.h"         // System-level definitions

#ifdef DS_MAILBOX_REMOTE

using namespace ds;

#include "Transmitter.h"      // Message transmitter
#include "PhysicalMailBox.h"  // Mailbox

// Configuration

/// Application identification
const char *System::app_name    PROGMEM = DS_APP_NAME "; Remote Module";
const char *System::app_version PROGMEM = DS_APP_VERSION;

//// I/O
static const int PIN_REED = 3;                   // No need to receive data; use GPIO3 (RX) for reed sensor
static const int PIN_HC12_SET = 0;               // Pin to control the configuration pin of HC-12
ADC_MODE(ADC_VCC);                               // Configure ADC to measure Vcc
Print* System::log = &Serial1;                   // Syslog on secondary UART (primary is occupied by transmitter)

//// Other
static const uint8_t MAILBOX_ID = 1;             // Mailbox identification number (1-15)

// Normally, no need to change below this line

// Global variables
static Transmitter transmitter(Serial, MAILBOX_ID, PIN_HC12_SET); // Transmitter
static PhysicalMailBox mailbox(MAILBOX_ID, PIN_REED); // Mailbox

void setup() {

  // First the system
  System::begin();

  // Initialize RF transmitter
  transmitter.begin();
  transmitter.wakeup();

  // Initialize mailbox
  mailbox.begin();

  // Send wakeup message
  transmitter << mailbox;
}

void loop() {

  // Check if it time to sleep. If door gets closed, go to sleep immediately, because next change to open will result in restart
  if (mailbox.doorClosed() || millis() >= mailbox.AWAKE_TIME) {
    System::log->printf(TIMED("%s.\n"), mailbox.doorClosed() ? "Door closed" : "Awake timeout reached");

    // Send status before sleeping
    mailbox.updateBattery();
    mailbox.setOnline(false);
    transmitter << mailbox;

    // Go to sleep
    transmitter.sleep();
    mailbox.sleep();
    System::log->printf(TIMED("Putting system to sleep\n"));
    ESP.deepSleep(0, WAKE_RF_DISABLED);
  }

  // Background processing
  System::update();
  mailbox.update();
}

#endif // DS_MAILBOX_REMOTE
