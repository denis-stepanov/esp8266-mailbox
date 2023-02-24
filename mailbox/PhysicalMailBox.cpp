/* DS mailbox automation
 * * Remote module
 * * * Mailbox implementation
 * (c) DNS 2020-2021
 */

#include "MySystem.h"         // System-level definitions

#ifdef DS_MAILBOX_REMOTE

#include "PhysicalMailBox.h"

using namespace ds;

static const uint8_t REED_OPEN = LOW;       // See schematic
static const uint16_t VCC_REF_000 = 3625;   // ADC reference reading at 0% battery. These numbers have been measured experimentally and depend on battery type and schematic
static const uint16_t VCC_REF_100 = 3925;   // ADC reference reading at 100% battery. Must be higher than VCC_REF_000

// Initialize mailbox
void PhysicalMailBox::begin() {
  System::log->printf(TIMED("Initializing mailbox... "));
  boot = System::getResetReason() != REASON_DEEP_SLEEP_AWAKE;
  online = true;

  // On return from deep sleep, load data from persistent memory
  if (!boot) {
    uint32_t rtc[2];
    if (System::getRTCMem(rtc, 0, sizeof(rtc) / sizeof(uint32_t))) {
      msg_num = rtc[0];
      battery = rtc[1];
    }
  }

  pinMode(pin_door, INPUT);
  update();
  updateBattery();
  System::log->println("OK");
}

// Update mailbox status
void PhysicalMailBox::update() {

  // Note that we do not need to debounce, as on door opening we restart, and on door closure we stop polling sensor after first bounce
  door = digitalRead(pin_door) == REED_OPEN;
}

// Update battery level
uint8_t PhysicalMailBox::updateBattery() {

  // Ignore readings during cold start, as there ESP starts with RF module on + cap charging, so measurements are very different
  if (!boot) {
    auto vcc = ESP.getVcc();

    // Normalize reading
    if (vcc < VCC_REF_000)
      vcc = VCC_REF_000;
    else
      if (vcc > VCC_REF_100)
        vcc = VCC_REF_100;

    const uint8_t battery_new = 100 * (vcc - VCC_REF_000) / (VCC_REF_100 - VCC_REF_000);
    if (battery != BATTERY_LEVEL_UNKNOWN) {

      // Vcc ADC has unstable readings, ranging +-20% in normal operation
      // Mitigate this with simple averaging by giving new value some predefined weight
      const int8_t diff = (battery_new - battery) / BATTERY_CHANGE_WEIGHT_COEFF;
      if (battery + diff < 0)
        battery = 0;
      else
        if (battery + diff > 100)
          battery = 100;
        else
          battery += diff;
    } else
      battery = battery_new;   // New baseline
  }
  return battery;
}

// Put mailbox to sleep
void PhysicalMailBox::sleep() const {
  System::log->printf(TIMED("Putting mailbox to sleep... "));
  uint32_t rtc[2];
  rtc[0] = msg_num;
  rtc[1] = battery;
  System::setRTCMem(rtc, 0, sizeof(rtc) / sizeof(uint32_t));
  System::log->println("OK");
}

#endif // DS_MAILBOX_REMOTE
