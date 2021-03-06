/* DS mailbox automation
 * * Local module
 * * * Mailbox Manager implementation
 * (c) DNS 2020-2021
 */

#include "MySystem.h"         // System log

#ifndef DS_MAILBOX_REMOTE

#include "MailBoxManager.h"

using namespace ds;

// Constructor
MailBoxManager::MailBoxManager(): alarm(ALARM_NONE) {}

// Collection destructor (normally never called)
MailBoxManager::~MailBoxManager() {
  for (auto mb : mailboxes)
    delete mb;
}

// Initialize mailboxes
void MailBoxManager::begin() {
  System::log->printf(TIMED("Initializing mailboxes... "));
  for (uint8_t i = MAILBOX_ID_MIN; i <= MAILBOX_ID_MAX; i++) {
    auto mailbox = VirtualMailBox::load(i);
    if (mailbox)
      mailboxes.push_front(mailbox);
    yield();
  }
  if (mailboxes.empty())
    System::log->println("none found");
  else
    System::log->printf("%d loaded\n", std::distance(mailboxes.begin(), mailboxes.end()));
}

// Regular check of mailboxes' status
void MailBoxManager::update(const bool force) {
  if (System::newHour() || force) {
    auto nok = false;

    for (auto mb : mailboxes)
      if(!mb->isOK())
        nok = true;

    if(nok)
      updateAlarm();
  }
}

// Find mailbox by ID. If not found, allow registering a new one
VirtualMailBox *MailBoxManager::getMailBox(const uint8_t mb_id, bool create) {
  VirtualMailBox *mailbox = nullptr;

  if (!mb_id)
    return mailbox;

  for (auto mb : mailboxes) {
    if (*mb == mb_id) {
      mailbox = mb;    // Found
      break;
    }
  }

  if (!mailbox && create) {

    // Register a new one
    mailbox = new VirtualMailBox(mb_id);
    if (mailbox) {
      mailboxes.push_front(mailbox);
      String lmsg = F("Registered new mailbox, id=");
      lmsg += mb_id;
      System::appLogWriteLn(lmsg, true);
    }
  }

  return mailbox;
}

// Find existing mailbox by ID
VirtualMailBox *MailBoxManager::operator[](const uint8_t mb_id) {
  return getMailBox(mb_id);
}

// Update mailbox from received message; create if not found
bool MailBoxManager::process(const MailBoxMessage &msg) {
  const auto mailbox = getMailBox(msg.getMailBoxID(), true);

  if (!mailbox) {
    System::log->printf(TIMED("Bad mailbox ID (%hhu) or problem creating a new one; no further action\n"), msg.getMailBoxID());
    return false;
  }

  // Update mailbox
  *mailbox = msg;

  // Update global alarm and its display
  updateAlarm();

  return true;
}

// Delete mailbox with a given ID
bool MailBoxManager::deleteMailBox(const uint8_t mb_id) {
  for (auto mb : mailboxes) {
    if (*mb == mb_id) {
      delete mb;
      mailboxes.remove(mb);
      return VirtualMailBox::forget(mb_id);
    }
  }
  return false; // Not found
}

// Update global alarm and its display with the latest status from mailboxes
void MailBoxManager::updateAlarm() {
  const auto alarm_prev = alarm;
  alarm = ALARM_NONE;
  for (auto mb : mailboxes) {
    auto a = mb->getAlarm();
    if (a > alarm)
      alarm = a;
  }
  if (alarm != alarm_prev) {

    // Signal the highest severity with LED
    switch (alarm) {
      case ALARM_NONE:          System::led.Off();                              break;
      case ALARM_BOOTED:        /* Signal the same way as absent */
      case ALARM_BATTERY:
      case ALARM_ABSENT:        System::led.Set(1);                             break;
      case ALARM_DOOR_FLIPPED:  System::led.On();                               break;
      case ALARM_DOOR_LEFTOPEN: System::led.Breathe(3000).Forever();            break;
      case ALARM_DOOR_OPEN:     System::led.Blink(250, 250).Forever();          break;
    }
  }
}

// Acknowledge global alarm. Returns the alarm acknowledged
mailbox_alarm MailBoxManager::acknowledgeAlarm(const String &via) {
  const auto alarm_ack = alarm;
  if (alarm != ALARM_NONE) {
    System::led.Off();
    String msg = F("Alarm \"");
    msg += VirtualMailBox::getAlarmStr(alarm);
    msg += F("\" acknowledged via ");
    msg += via;
    System::appLogWriteLn(msg, true);
    alarm = ALARM_NONE;
    for (auto mb : mailboxes)
      mb->resetAlarm();
  }
  return alarm_ack;
}

// Print mailboxes table in HTML
//// This method should have been const, but LinkedList is not sufficiently well written for that
void MailBoxManager::printHTML(String& buf) {
  buf += F("<form action=\"/ack\">\n"
           "<span style=\"font-size: 3cm;\">");
  buf += VirtualMailBox::getAlarmIcon(alarm);
  buf += F("</span>\n<p>Global status:&nbsp;&nbsp;");
  buf += VirtualMailBox::getAlarmStr(alarm, true);
  buf += F("&nbsp;&nbsp;<input type=\"submit\" value=\"Acknowledge\"");
  if (alarm == ALARM_NONE)
    buf += F(" disabled=\"true\"");
  buf += F("/></p>\n</form>\n"
    "<table border=\"1\" cellpadding=\"3\" cellspacing=\"0\" style=\"font-family: monospace; border-collapse: collapse;\">\n"
    "<tr><th title=\"ID\">&#x1f4ec;</th><th title=\"Label\">&#x1f3f7;</th><th title=\"Status\">&#x1f6a9;</th>"
    "<th title=\"Battery\">&#x1f50b;</th><th title=\"Radio Reliability\">&#x1f4f6;</th><th title=\"Last Contact\">&#x1f557;</th></tr>\n");
  if (mailboxes.empty())
    buf += F("<tr><td colspan=\"6\" style=\"text-align: center\">- No mailboxes have reported so far -</tr>\n");
  else
    for (auto mb : mailboxes)
      buf << *mb;
  buf += F("</table>\n");
}

// Print mailboxes table in text
//// This method should have been const, but LinkedList is not sufficiently well written for that
void MailBoxManager::printText(String& buf) {
  if (mailboxes.empty())
    buf += F("No mailboxes have reported so far");
  else
    for (auto mb : mailboxes)
      mb->printText(buf);
}

// HTML printout helper
String& operator<<(String& html_buf, MailBoxManager& mbm) {
  mbm.printHTML(html_buf);
  return html_buf;
}

#endif // !DS_MAILBOX_REMOTE
