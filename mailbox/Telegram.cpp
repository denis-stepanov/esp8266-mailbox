/* DS mailbox automation
 * * Local module
 * * * Telegram implementation
 * (c) DNS 2020-2023
 */

#include "MySystem.h"               // LED control; network status

#if defined(DS_SUPPORT_TELEGRAM) && !defined(DS_MAILBOX_REMOTE)

#include "Telegram.h"
#include "MailBoxManager.h"         // Mailbox manager

using namespace ds;

// Server data providers
extern MailBoxManager mailbox_manager;     // Mailbox manager instance

// Telegram poll interval
static const unsigned int POLL_INTERVAL = 10;  // s

// Settings file
static const char *TG_CONF_FILE_NAME PROGMEM = "/telegram.cfg";

// Constructor
Telegram::Telegram(): bot(token, client), timer((char *)nullptr, POLL_INTERVAL, std::bind(&Telegram::update, this), false),
    active(false), boot_reported(false), bounce_reported(false), update_in_progress(false) {
  client.setInsecure();    // See https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot/issues/118
}

// Return bot token
const String& Telegram::getToken() const {
  return token;
}

// Set bot token
void Telegram::setToken(const String& new_token) {
  token = new_token;
  token.trim();
  bot.updateToken(token);
}

// Return chat ID
const String& Telegram::getChatID() const {
  return chat_id;
}

// Set chat ID
void Telegram::setChatID(const String& new_chat_id) {
  chat_id = new_chat_id;
  chat_id.trim();
}

// Begin operations
void Telegram::begin() {

  // Load configuration if present
  if (load())
    System::log->printf(TIMED("%s: Telegram configured, %sactive\n"), TG_CONF_FILE_NAME, active ? "" : "in");
}

// Load configuration from disk
bool Telegram::load() {
  auto file = System::fs.open(TG_CONF_FILE_NAME, "r");
  if (!file)
    return false;
  setToken(file.readStringUntil('\n'));
  setChatID(file.readStringUntil('\n'));
  const auto is_active = file.parseInt();
  file.close();
  if (is_active)
    activate();
  else
    deactivate();
  return true;
}

// Save configuration to disk
bool Telegram::save(const String& new_token, const String& new_chat_id, bool new_active) {
  token = new_token;
  chat_id = new_chat_id;
  if (new_active)
    activate();
  else
    deactivate();
  auto file = System::fs.open(TG_CONF_FILE_NAME, "w");
  if (!file) {
    System::log->printf(TIMED("Error saving Telegram configuration\n"));
    return false;
  }
  file.println(token);
  file.println(chat_id);
  file.println(active ? 1 : 0);
  file.close();
  return true;
}

// Activate service
void Telegram::activate() {
  if (!active) {
    active = true;
    timer.arm();
  }
}

// Deactivate service
void Telegram::deactivate() {
  if (active) {
    timer.disarm();
    active = false;
    boot_reported = false;
    bounce_reported = false;
    update_in_progress = false;
  }
}

// Return true if service is active
bool Telegram::isActive() const {
  return active;
}

// Send message with logging
bool Telegram::sendMessage(const String& chat_id, const String& to, const String& cmd, const String& msg) {
  if (System::networkIsConnected()) {
    if (token.length() && chat_id.length()) {
      System::log->printf(TIMED("Serving Telegram command \""));
      System::log->print(cmd);
      System::log->print(F("\" to "));
      System::log->print(to);
      System::log->println(chat_id[0] == '-' ? F(" in public") : F(" in private"));
      return bot.sendMessage(chat_id, msg, F("Markdown"));
    } else {
      System::log->printf(TIMED("Telegram message not sent: invalid credentials\n"));
      return false;
    }
  } else {
    System::log->printf(TIMED("Telegram message not sent: network is down\n"));
    return false;
  }
}

// Send test message
bool Telegram::sendTest(const String& new_token, const String& new_chat_id) {

  // Do not check "active" flag here, as this method is called intentionally for testing

  if (System::networkIsConnected()) {

    if (token != new_token)
      bot.updateToken(new_token);

    // Do not timestamp test message
    const auto ret = bot.sendMessage(new_chat_id, F("Hi there! This is a test message from the mailbox app."));

    if (token != new_token)
      bot.updateToken(token);

    return ret;
  } else {
    System::log->printf(TIMED("Telegram message not sent: network is down\n"));
    return false;
  }
}

// Send boot notification
bool Telegram::sendBoot() {
  if (!active)
    return false;

  if (System::networkIsConnected()) {

    // Do not timestamp this, as usually when this is sent, time is not synchronized yet
    return bot.sendMessage(chat_id, F("Mailbox receiver booted"));
  } else {
    System::log->printf(TIMED("Telegram message not sent: network is down\n"));
    return false;
  }
}

// Send low battery notification
bool Telegram::sendBatteryLow(const VirtualMailBox& mb) {
  if (!active)
    return false;

  if (System::networkIsConnected()) {

    char time_str[7];
    const auto t = System::getTime();
    strftime(time_str, sizeof(time_str), "%H:%M ", localtime(&t));
    String output(time_str);
    output += mb.getAlarmIcon(ALARM_BATTERY);
    output += F(" Mailbox ");
    output += mb.getName();
    output += F(" is low on battery (");
    output += mb.getBattery();
    output += F("%)");

    return bot.sendMessage(chat_id, output);
  } else {
    System::log->printf(TIMED("Telegram message not sent: network is down\n"));
    return false;
  }
}

// Send lost event notification
bool Telegram::sendLostEvent(uint16_t num) {
  if (!active)
    return false;

  if (System::networkIsConnected()) {
    if (num) {
      String msg = F("Detected loss of ");
      msg += num;
      msg += F(" event");
      msg += num % 10 == 1 && num != 11 ? F("") : F("s");
      msg += " beforehand";
      return bot.sendMessage(chat_id, msg);
    } else
      return false;
  } else {
    System::log->printf(TIMED("Telegram message not sent: network is down\n"));
    return false;
  }
}

// Send event notification
bool Telegram::sendEvent(const VirtualMailBox& mb, uint16_t remote_time) {
  if (!active)
    return false;

  if (System::networkIsConnected()) {
    auto alarm = mb.getAlarm();

    // Do not report boot twice
    if (alarm == ALARM_BOOTED && boot_reported) {
      boot_reported = false;
      return true;
    }
    boot_reported = false;

    // Do not report another closure if door bounced (i.e., found closed on wake up)
    if (alarm == ALARM_DOOR_FLIPPED && bounce_reported) {
      bounce_reported = false;
      return true;
    }
    bounce_reported = false;

    char time_str[7];
    const auto t = System::getTime();
    strftime(time_str, sizeof(time_str), "%H:%M ", localtime(&t));
    String output(time_str);
    output += mb.getAlarmIcon();

    // Show action text
    output += F(" Mailbox ");
    output += mb.getName();
    switch (alarm) {
      case ALARM_NONE:
        return true;       // Nothing to send

      case ALARM_BOOTED:
        output += F(" rebooted");
        boot_reported = true;
        break;

      case ALARM_BATTERY:
        output += F(" is low on battery (");
        output += mb.getBattery();
        output += F("%)");
        break;

      case ALARM_ABSENT: {
          const auto days = (System::getTime() - mb.getLastSeen()) / (24 * 60 * 60);
          output += F(" haven't reported back for ");
          output += days;
          output += F(" day");
          if (days / 1000 % 10 != 1 || days / 1000 == 11)
            output += F("s");
        }
        break;

      case ALARM_DOOR_FLIPPED:
        if (remote_time / 1000) {
          output += F(" closed after ");
          output += remote_time / 1000;  // Assuming opening was within 1s from boot
          output += F(" second");
          if (remote_time / 1000 % 10 != 1 || remote_time / 1000 == 11)
            output += F("s");
        } else {
          output += F(" bounced");
          bounce_reported = true;
        }
        break;

      case ALARM_DOOR_LEFTOPEN:
        output += F(" still opened after ");
        output += remote_time / 1000;
        output += F(" second");
        if (remote_time / 1000 % 10 != 1 || remote_time / 1000 == 11)
          output += F("s");
        output += F(". Closure will not be reported");
        break;

      case ALARM_DOOR_OPEN:
        output += F(" opened");
        break;
    }

    String keyboard(F("["));
    mailbox_manager.printTelegramKeyboard(keyboard); // Add buttons for individual mailboxes
    keyboard += F(",[\"/ack\",\"/status\"],[\"/ack +status\"]]"); // Add global buttons
    return bot.sendMessageWithReplyKeyboard(chat_id, output, "", keyboard, true);
  } else {
    System::log->printf(TIMED("Telegram message not sent: network is down\n"));
    return false;
  }
}

// Process incoming commands
void Telegram::update() {
  if (!active || !System::networkIsConnected() || update_in_progress)
    return;

  update_in_progress = true;
  const auto n_msg = bot.getUpdates(bot.last_message_received + 1);
  update_in_progress = false;
  for(int i = 0; i < n_msg; i++) {
    const telegramMessage& input = bot.messages[i];

    if (input.text.startsWith("/ack")) {
      String via = F("Telegram by ");
      via += input.from_name;
      const auto alarm = mailbox_manager.acknowledgeAlarm(via);
      String reply;
      if (alarm != ALARM_NONE) {
        reply = F("Acknowledged \"");
        reply += VirtualMailBox::getAlarmStr(alarm);
        reply += F("\" alarm");
      } else
        reply = F("Nothing to acknowledge");
      if (input.text.endsWith(F(" +status"))) {
        reply += "\n";
        mailbox_manager.printText(reply);
      }
      sendMessage(input.chat_id, input.from_name, input.text, reply);
      continue;
    }

    if (input.text == F("/status")) {
      String output;
      mailbox_manager.printText(output);
      sendMessage(input.chat_id, input.from_name, input.text, output);
      continue;
    }

    if (input.text == F("/help")) {
      String output = F(
        "Supported commands:\n"
        "/ack    - acknowledge mailbox event\n"
        "/status - show mailbox status\n"
        "/help   - show help\n"
        );
      sendMessage(input.chat_id, input.from_name, input.text, output);
      continue;
    }
  }
}

#endif // DS_SUPPORT_TELEGRAM && !DS_MAILBOX_REMOTE
