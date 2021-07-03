/* DS mailbox automation
 * * Local module
 * * * Telegram implementation
 * (c) DNS 2020-2021
 */

#include "MySystem.h"               // LED control

#if defined(DS_SUPPORT_TELEGRAM) && !defined(DS_MAILBOX_REMOTE)

#include "Telegram.h"
#include "MailBoxManager.h"         // Mailbox manager

using namespace ds;

// Server data providers
extern MailBoxManager mailbox_manager;     // Mailbox manager instance

// Add your Telegram credentials here
const char *Telegram::token      PROGMEM = "nnnnnnnnnn:xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";  // Bot token
const char *Telegram::chat_id    PROGMEM =
#ifdef DS_DEVBOARD
                                           "nnnnnnnnn"                                        // ID of private chat with developer
#else
                                          "-nnnnnnnnn"                                        // ID of group chat where bot reports
#endif // DS_DEVBOARD
;
static const unsigned int POLL_INTERVAL = 10;  // s

// Constructor
Telegram::Telegram(): bot(token, client), timer((char *)nullptr, POLL_INTERVAL, std::bind(&Telegram::update, this)),
    boot_reported(false), bounce_reported(false) {
  client.setInsecure();    // See https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot/issues/118
}

// Send message with logging
bool Telegram::sendMessage(const String& chat_id, const String& to, const String& cmd, const String& msg) {
  System::log->printf(TIMED("Serving Telegram command \""));
  System::log->print(cmd);
  System::log->print(F("\" to "));
  System::log->print(to);
  System::log->println(chat_id[0] == '-' ? F(" in public") : F(" in private"));
  return bot.sendMessage(chat_id, msg, F("Markdown"));
}

// Send boot notification
bool Telegram::sendBoot() {

  // Do not timestamp this, as usually when this is sent, time is not synchronized yet
  return bot.sendMessage(chat_id, F("Mailbox receiver booted"));
}

// Send lost event notification
bool Telegram::sendLostEvent(uint16_t num) {
  if (num) {
    String msg = F("Detected loss of ");
    msg += num;
    msg += F(" event");
    msg += num % 10 == 1 && num != 11 ? F("") : F("s");
    msg += " beforehand";
    return bot.sendMessage(chat_id, msg);
  } else
    return false;
}

// Send event notification
bool Telegram::sendEvent(const VirtualMailBox& mb, uint16_t remote_time) {
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

  return bot.sendMessageWithReplyKeyboard(chat_id, output, "", F("[[\"/ack\",\"/status\"],[\"/ack +status\"]]"), true);
}

// Process incoming commands
void Telegram::update() {
  const auto n_msg = bot.getUpdates(bot.last_message_received + 1);
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
