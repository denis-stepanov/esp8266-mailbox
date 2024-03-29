/* DS mailbox automation
 * * Local module
 * (c) DNS 2020-2023
 */

#include "MySystem.h"         // System-level definitions

#ifndef DS_MAILBOX_REMOTE

#include "Receiver.h"         // Message receiver
#include "MailBoxManager.h"   // Mailbox manager
#include "GoogleAssistant.h"  // Google interface
#ifdef DS_SUPPORT_TELEGRAM
#include "Telegram.h"         // Telegram interface
#endif // DS_SUPPORT_TELEGRAM

using namespace ace_button;
using namespace ds;

// Configuration

/// Application identification
const char *System::app_name    PROGMEM = DS_APP_NAME "; Local Module"
#ifdef DS_DEVBOARD
                                          " (DEV)"
#endif // DS_DEVBOARD
;
const char *System::app_version PROGMEM = DS_APP_VERSION;
const char *System::app_url     PROGMEM = DS_APP_URL;

//// I/O
#ifndef DS_DEVBOARD
Print* System::log = &Serial1;                   // Syslog on secondary UART (primary is occupied by receiver)
#endif // !DS_DEVBOARD

//// Network
const char *System::hostname PROGMEM = "Mailbox" // <hostname>.local in the local network. Also SSID of the temporary network for Wi-Fi configuration
#ifdef DS_DEVBOARD
                                       "-dev"
#endif // DS_DEVBOARD
;

// Normally, no need to change below this line

// Global objects
static Receiver receiver;                        // RF receiver
MailBoxManager mailbox_manager;                  // Mailbox manager
GoogleAssistant google_assistant;                // Google interface
#ifdef DS_SUPPORT_TELEGRAM
Telegram telegram;                               // Telegram interface
#endif // DS_SUPPORT_TELEGRAM
static auto check_degraded = false;              // Indicates when it is safe to run mailbox status check on boot
#ifdef DS_DEVBOARD
extern bool recv_message_emulated;               // Flag to emulate message arrival
#endif //DS_DEVBOARD

// System hooks
#ifdef DS_DEVBOARD
//// Button initialization
static void handleButtonInit() {
  System::button.getButtonConfig()->setFeature(ButtonConfig::kFeatureDoubleClick);
  System::button.getButtonConfig()->setFeature(ButtonConfig::kFeatureSuppressClickBeforeDoubleClick); // To distinguish between click and double click
  System::button.getButtonConfig()->setClickDelay(300);         // Defaults are a bit uncomfortable with the Witty Cloud button
  System::button.getButtonConfig()->setDoubleClickDelay(600);
}
#endif // DS_DEVBOARD

//// Button event handler
static void handleButtonEvent(AceButton* /* button */, uint8_t event_type, uint8_t /* button_state */) {
  switch (event_type) {

    case                                  // Alarm acknowledgement
#ifdef DS_DEVBOARD
      AceButton::kEventClicked            // In dev env, react on click instead, to allow for double click
#else
      AceButton::kEventPressed
#endif
      :
      mailbox_manager.acknowledgeAlarm(F("button"));
      break;

#ifdef DS_DEVBOARD
    case AceButton::kEventDoubleClicked:  // Emulate incoming event
      recv_message_emulated = true;
      break;
#endif // DS_DEVBOARD
  }
}

//// Time sync handler
static void handleTimeSync() {
  static time_sync_t time_sync_status = TIME_SYNC_NONE;  // Time sync status

  // Once time gets initialized, activate check for mailboxes in degraded condition (from past knowledge)
  // We cannot invoke the check directly from here, as it might end up calling Telegram while another Telegram request is in progress
  const auto time_sync_status_new = System::getTimeSyncStatus();
  if (time_sync_status == TIME_SYNC_NONE && time_sync_status_new != TIME_SYNC_NONE) {
    check_degraded = true;
    time_sync_status = time_sync_status_new;
  }
}

// Timer handler
static void handleAbsTimer(const TimerAbsolute* timer) {
  const auto& action = timer->getAction();

  if (action.startsWith("signal absent msg for mb_id=")) {

    // This would be far easier if timer could store a back reference to mailbox; see https://github.com/denis-stepanov/esp-ds-system/issues/61
    const auto mb = mailbox_manager[action.substring(28).toInt()];
    if (mb)
      mb->timeout();
  }

#ifdef DS_SUPPORT_TELEGRAM
  if (action == "poll TG") {
    telegram.update();
  }
#endif // DS_SUPPORT_TELEGRAM

}

//// Install hooks
#ifdef DS_DEVBOARD
void (*System::onButtonInit)() = handleButtonInit;
#endif // DS_DEVBOARD
void (*System::onButtonPress)(AceButton*, uint8_t, uint8_t) = handleButtonEvent;
void (*System::onTimeSync)() = handleTimeSync;
void (*System::timerHandler)(const TimerAbsolute*) = handleAbsTimer;
  
void setup() {

  // First the system. Diagnostics are printed from inside
  System::begin();

  // Initialize mailbox manager
  mailbox_manager.begin();

  // Initialize RF receiver
  receiver.begin();

  // Load Google configuration
  google_assistant.begin();

#ifdef DS_SUPPORT_TELEGRAM
  // Load Telegram configuration
  telegram.begin();

  // Notify on Telegram about reboot
  telegram.sendBoot();
#endif // DS_SUPPORT_TELEGRAM
}

void loop() {

  // Check if we can run mailbox status check after boot
  if (check_degraded) {
    mailbox_manager.update(true);
    check_degraded = false;
  }

  // Check for incoming message
  if (receiver.messageAvailable())
    mailbox_manager.process(receiver.getMessage());

  // Background processing
  System::update();
  receiver.update();
  mailbox_manager.update();
}

#endif // !DS_MAILBOX_REMOTE
