/* DS mailbox automation
 * * Local module
 * * * Mailbox implementation
 * (c) DNS 2020-2023
 */

#include "MySystem.h"         // System settings

#ifndef DS_MAILBOX_REMOTE

#include "VirtualMailBox.h"
#include "MailBoxManager.h"   // Mailbox manager
#ifdef DS_SUPPORT_TELEGRAM
#include "Telegram.h"         // Telegram interface
#endif // DS_SUPPORT_TELEGRAM
#ifdef DS_SUPPORT_GOOGLE_ASSISTANT
#include "GoogleAssistant.h"  // Google interface
#endif // DS_SUPPORT_GOOGLE_ASSISTANT

using namespace ds;

extern MailBoxManager mailbox_manager;      // Mailbox manager instance
#ifdef DS_SUPPORT_TELEGRAM
extern Telegram telegram;                   // Telegram interface
#endif // DS_SUPPORT_TELEGRAM
#ifdef DS_SUPPORT_GOOGLE_ASSISTANT
extern GoogleAssistant google_assistant;    // Google interface
#endif // DS_SUPPORT_GOOGLE_ASSISTANT

static const char *FILE_PREFIX PROGMEM = "/mailbox"; // Configuration file prefix
static const char *FILE_EXT PROGMEM = ".cfg";        // Configuration file extension

// Constructor
VirtualMailBox::VirtualMailBox(const uint8_t _id, const String _label, const uint8_t _battery, const time_t _last_seen) :
  MailBox(_id, _label, _battery), last_seen(_last_seen), msg_recv(0), alarm(ALARM_NONE),
  timer((char *)nullptr, (AWAKE_TIME + 5000 /* slack 5s */) / 1000.0, std::bind(&VirtualMailBox::timeout, this), false, false),
  g_opening_reported(false), low_battery_reported(false) {
}


// Return the last report time
time_t VirtualMailBox::getLastSeen() const {
  return last_seen;
}

// Set the last report time. 0 means current time
void VirtualMailBox::setLastSeen(const time_t t) {
  if (t)
    last_seen = t;
  else
    if (System::getTimeSyncStatus() != TIME_SYNC_NONE)
      last_seen = System::getTime();
}

// Return radio link reliability (%). -1 == unknown
int8_t VirtualMailBox::getRadioReliability() const {
  return msg_count ? 100 * msg_recv / msg_count : -1;
}

// Return mailbox alarm
mailbox_alarm VirtualMailBox::getAlarm() const {
  return alarm;
}

// Return mailbox alarm as string (possibly, HTMLized)
String VirtualMailBox::getAlarmStr(const bool html) const {
  return getAlarmStr(alarm, html);
}

// Return mailbox alarm as string (static version)
String VirtualMailBox::getAlarmStr(const mailbox_alarm a, const bool html) {
  String str;

  if (html) {
    str += F("<span class=\"alarm");
    str += a;
    str += F("\">");
  }
  switch (a) {
    case ALARM_NONE:          str += F("No event");        break;
    case ALARM_BOOTED:        str += F("Rebooted");        break;
    case ALARM_BATTERY:       str += F("Low battery");     break;
    case ALARM_ABSENT:        str += F("Absent");          break;
    case ALARM_DOOR_FLIPPED:  str += F("Door flipped");    break;
    case ALARM_DOOR_LEFTOPEN: str += F("Door left open");  break;
    case ALARM_DOOR_OPEN:     str += F("Door opened NOW"); break;
  }
  if (html)
    str += F("</span>");
  return str;
}

// Return alarm icon
String VirtualMailBox::getAlarmIcon() const {
  return getAlarmIcon(alarm);
}

// Return alarm icon (static version)
String VirtualMailBox::getAlarmIcon(const mailbox_alarm a) {
  String icon;
  switch (a) {
    case ALARM_NONE:          icon = "\xf0\x9f\x93\xaa"; break;  // UTF-8 'CLOSED MAILBOX WITH LOWERED FLAG'
    case ALARM_BOOTED:        icon = "\xf0\x9f\x92\xa5"; break;  // UTF-8 'COLLISION SYMBOL'
    case ALARM_BATTERY:       icon = "\xf0\x9f\x94\x8b"; break;  // UTF-8 'BATTERY'
    case ALARM_ABSENT:        icon = "\xf0\x9f\x9a\xab"; break;  // UTF-8 'NO ENTRY SIGN'
    case ALARM_DOOR_FLIPPED:  icon = "\xf0\x9f\x93\xab"; break;  // UTF-8 'CLOSED MAILBOX WITH RAISED FLAG'
    case ALARM_DOOR_LEFTOPEN: icon = "\xf0\x9f\x93\xac"; break;  // UTF-8 'OPEN MAILBOX WITH RAISED FLAG'
    case ALARM_DOOR_OPEN:     icon = "\xf0\x9f\x93\xad"; break;  // UTF-8 'OPEN MAILBOX WITH LOWERED FLAG'
  }
  return icon;
}

// Update mailbox alarm
void VirtualMailBox::updateAlarm() {
  alarm = boot ? ALARM_BOOTED : (door ? (alarm == ALARM_DOOR_OPEN ? ALARM_DOOR_LEFTOPEN : ALARM_DOOR_OPEN) : ALARM_DOOR_FLIPPED);
}

// Reset mailbox alarm
void VirtualMailBox::resetAlarm() {
  alarm = ALARM_NONE;
  low_battery_reported = false;
}

// Return false in degraded conditions (battery low or mailbox absent)
bool VirtualMailBox::isOK() {
  auto is_ok = true;

  // Check if mailbox has not been seen for a while
  if (last_seen && alarm != ALARM_ABSENT && System::getTimeSyncStatus() != TIME_SYNC_NONE &&
       (unsigned long)(System::getTime() - last_seen) >= ABSENCE_TIME) {
    is_ok = false;
    alarm = ALARM_ABSENT;    // Override possible stale higher level alarm
    String lmsg = F("Marking mailbox ");
    lmsg += getName();
    lmsg += F(" as absent");
    System::appLogWriteLn(lmsg, true);
#ifdef DS_SUPPORT_TELEGRAM
    telegram.sendEvent(*this);
#endif
  }

  // Check is mailbox is low on battery
  if (alarm < ALARM_BATTERY && getBattery() <= BATTERY_LEVEL_LOW) {
    is_ok = false;
    alarm = ALARM_BATTERY;
  }

  if (getBattery() <= BATTERY_LEVEL_LOW && !low_battery_reported) {
    String lmsg = F("Mailbox ");
    lmsg += getName();
    lmsg += F(" is low on battery");
    System::appLogWriteLn(lmsg, true);
#ifdef DS_SUPPORT_TELEGRAM
    telegram.sendBatteryLow(*this);
#endif
    low_battery_reported = true;
  }

  return is_ok;
}

// Print mailbox status in HTML
void VirtualMailBox::printHTML(String& buf) const {
  buf += F("<tr><td><a href=\"/mailbox?id=");
  buf += id;
  buf += F("\">");
  buf += id;
  buf += F("</a></td><td>");
  buf += label;
  buf += F("</td><td>");
  buf += getAlarmIcon();
  buf += F(" ");
  buf += getAlarmStr(true);
  buf += F("</td><td>");
  const auto bl = getBattery();
  if (bl != BATTERY_LEVEL_UNKNOWN) {
    if (bl <= BATTERY_LEVEL_LOW)
      buf += F("<span class=\"alarm\">");
    buf += bl;
    buf += F("%");
    if (bl <= BATTERY_LEVEL_LOW)
      buf += F("</span>");
  }
  buf += F("</td><td>");
  const auto rr = getRadioReliability();
  if (rr != -1) {
    if (rr <= RADIO_RELIABILITY_BAD)
      buf += F("<span class=\"alarm\">");
    buf += rr;
    buf += F("%");
    if (rr <= RADIO_RELIABILITY_BAD)
      buf += F("</span>");
  }
  buf += F("</td><td>");
  char time_str[19];
  strftime(time_str, sizeof(time_str), "%a %d-%b %H:%M", localtime(&last_seen));
  auto t = System::getTime();
  if (t && last_seen && (unsigned long)(t - last_seen) >= ABSENCE_TIME)
    buf += F("<span class=\"alarm\">");
  buf += time_str;
  if (t && last_seen && (unsigned long)(t - last_seen) >= ABSENCE_TIME)
    buf += F("</span>");
  buf += F("</td></tr>\n");
}

// Print mailbox status in text
void VirtualMailBox::printText(String& buf) const {
  buf += F("Mailbox ");
  buf += getName();
  buf += F(": ");
  buf += getAlarmIcon();
  buf += F(" ");
  buf += getAlarmStr();
  const auto bl = getBattery();
  if (bl != BATTERY_LEVEL_UNKNOWN) {
    buf += F(", \xf0\x9f\x94\x8b ");   // UTF-8 'BATTERY'
    if (bl <= BATTERY_LEVEL_LOW)
      buf += F("*");       // * = markdown 'bold'
    buf += bl;
    buf += F("%");
    if (bl <= BATTERY_LEVEL_LOW)
      buf += F("*");
  }
  const auto rr = getRadioReliability();
  if (rr != -1) {
    buf += F(", \xf0\x9f\x93\xb6 ");   // UTF-8 'ANTENNA WITH BARS'
    if (rr <= RADIO_RELIABILITY_BAD)
      buf += F("*");
    buf += rr;
    buf += F("%");
    if (rr <= RADIO_RELIABILITY_BAD)
      buf += F("*");
  }
  buf += F(", \xf0\x9f\x95\x97 ");     // UTF-8 'CLOCK FACE EIGHT OCLOCK'
  char time_str[19];
  strftime(time_str, sizeof(time_str), "%a %d-%b %H:%M", localtime(&last_seen));
  auto t = System::getTime();
  if (t && last_seen && (unsigned long)(t - last_seen) >= ABSENCE_TIME)
    buf += F("*");
  buf += time_str;
  if (t && last_seen && (unsigned long)(t - last_seen) >= ABSENCE_TIME)
    buf += F("*");
  buf += "\n";
}

// Return configuration file name (static version)
String VirtualMailBox::getConfFileName(const uint8_t id) {
  String file_name = FILE_PREFIX;
  file_name += id;
  file_name += FILE_EXT;
  return file_name;
}

// Return configuration file name
String VirtualMailBox::getConfFileName() const {
  return getConfFileName(id);
}

// Save mailbox information to disk
void VirtualMailBox::save() const {
  auto file = System::fs.open(getConfFileName(), "w");
  if (!file) {
    System::log->printf(TIMED("Error saving configuration for mailbox=%hhu\n"), id);
    return;
  }
  file.println(label);
  file.println(last_seen);
  file.println(battery);
  file.close();
}

// Initialize mailbox with information on disk
VirtualMailBox *VirtualMailBox::load(const uint8_t id) {
  auto file = System::fs.open(getConfFileName(id), "r");
  if (!file)
    return nullptr;
  auto label = file.readStringUntil('\n');
  label.trim();
  time_t last_seen = file.parseInt();
  uint8_t battery = file.parseInt();
  file.close();
  return new VirtualMailBox(id, label, battery, last_seen);
}

// Remove mailbox information from disk
bool VirtualMailBox::forget(const uint8_t id) {
  return System::fs.remove(getConfFileName(id));
}

// Update mailbox from message data
static const uint16_t LOST_MESSAGE_MAX = 1000; // Threshold after which we consider our counter to be out of sync
VirtualMailBox& VirtualMailBox::operator=(const MailBoxMessage& msg) {

  // Check for lost messages. Messages lost during boot are exempted from the check
  String lmsg;
  auto msg_num_cur = msg_num;
  uint16_t msg_lost = 0;
  const auto msg_num_new = msg.getMessageNumber();
  if (msg_num_cur != MESSAGE_NUMBER_UNKNOWN && !msg.getBoot()) {
    MailBoxMessage::getNextMessageNumber(msg_num_cur);
    msg_lost = msg_num_new - msg_num_cur;   // Overflow-safe
    if (msg_lost <= LOST_MESSAGE_MAX) {
      msg_count += msg_lost;
      if (msg_lost) {
        lmsg = F("Lost ");
        lmsg += msg_lost;
        lmsg += F(" message(s)!");
        System::appLogWriteLn(lmsg);
      }
    } else {
      System::appLogWriteLn(F("Message counter is out of sync; resetting"));
      msg_lost = 0;
    }
  }

  // Update mailbox fields
  msg_count++;
  msg_recv++;
  setLastSeen();
  msg_num = msg_num_new;
  boot = msg.getBoot();
  online = msg.getOnline();
  const auto battery_new = msg.getBattery();
  if (battery_new != BATTERY_LEVEL_UNKNOWN)
    battery = battery_new;
  door = msg.getDoor();
  updateAlarm();
  save();

  // Report in the log
  lmsg = F("(");
  lmsg += msg_num;
  lmsg += F(") Mailbox ");
  lmsg += getName();

  //// Regular "cumulative status" alarm string is not really good for momentary logging, so make a separate interpretation here
  const auto remote_time = msg.getTime();
  switch (alarm) {
    case ALARM_NONE:       /* Never happens here */         break;
    case ALARM_BOOTED:        lmsg += online ? F(" rebooted") : F(" sleeping after reboot"); break;
    case ALARM_BATTERY:    /* Never happens here */         break;
    case ALARM_ABSENT:     /* Never happens here */         break;
    case ALARM_DOOR_FLIPPED:

      // Similar code is in Telegram section
      if (remote_time / 1000) {
        lmsg += F(" closed after ");
        lmsg += remote_time / 1000;  // Assuming opening was within 1s from boot
        lmsg += F(" second");
        if (remote_time / 1000 % 10 != 1 || remote_time / 1000 == 11)
          lmsg += F("s");
      } else
        lmsg += F(" bounced");
      break;

    case ALARM_DOOR_LEFTOPEN: lmsg += F(" door left open"); break;
    case ALARM_DOOR_OPEN:     lmsg += F(" door opened");    break;
  }
  lmsg += F("; battery ");
  if (battery_new == BATTERY_LEVEL_UNKNOWN)
    lmsg += F("---");
  else
    lmsg += battery;
  lmsg += F("%");
  System::appLogWriteLn(lmsg);

#ifdef DS_SUPPORT_TELEGRAM
  // Send event notification to cloud
  telegram.sendEvent(*this, remote_time);
#endif // DS_SUPPORT_TELEGRAM
#ifdef DS_SUPPORT_GOOGLE_ASSISTANT
  if (g_opening_reported)
    g_opening_reported = false;
  else
    if (alarm >= ALARM_DOOR_FLIPPED) {
      if (alarm == ALARM_DOOR_FLIPPED && !(remote_time / 1000)) {

        // Mailbox bounced; skip reporting
        g_opening_reported = true;
      } else {
        String gmsg(F("Mailbox "));
        gmsg += getName();
        gmsg += F(" opened");
        g_opening_reported = google_assistant.broadcast(gmsg) && online;
      }
    }
#endif // DS_SUPPORT_GOOGLE_ASSISTANT

#ifdef DS_SUPPORT_TELEGRAM
  // Each mailbox event is a pair of messages with numbers odd (start) + even (finish)
  // Hence receiving new odd message after 2+ missing messages mean missed event(s), while
  //      receiving new even message after 3+ missing messages mean missed event(s)
  uint32_t lost_event = msg_num_cur == MESSAGE_NUMBER_UNKNOWN || msg_lost ? (msg_lost - (msg_num_new + 1) % 2) / 2 : 0;
  if (lost_event)
    telegram.sendLostEvent(lost_event);
#endif // DS_SUPPORT_TELEGRAM

  // Set a timeout handler in the case the second message never arrives
  if (online)
    timer.arm();
  else
    timer.disarm();

  return *this;
}

// Message timeout handler
void VirtualMailBox::timeout() {

  // Assume the message has been sent but did not arrive
  MailBoxMessage::getNextMessageNumber(msg_num);
  msg_count++;

  String lmsg = F("Mailbox ");
  lmsg += getName();
  lmsg += F(" door closure event timed out; potentially lost 1 message");
  System::appLogWriteLn(lmsg, true);

  // In the case alarm has been acknowledged before timeout, do not reinstate it
  if (alarm != ALARM_NONE) {
    updateAlarm();
    mailbox_manager.updateAlarm();
  }

#ifdef DS_SUPPORT_GOOGLE_ASSISTANT
  // Prepare for the next event
  g_opening_reported = false;
#endif // DS_SUPPORT_GOOGLE_ASSISTANT
}

// HTML printout helper
String& operator<<(String& html_buf, VirtualMailBox& mb) {
  mb.printHTML(html_buf);
  return html_buf;
}

#endif // !DS_MAILBOX_REMOTE
