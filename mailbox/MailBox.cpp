/* DS mailbox automation
 * * Basic mailbox implementation
 * (c) DNS 2020-2022
 */

#include "MailBox.h"

using namespace ds;

// Return mailbox ID
uint8_t MailBox::getID() const {
  return id;
}

// Return mailbox label
const String& MailBox::getLabel() const {
  return label;
}

// Set mailbox label
void MailBox::setLabel(const String& new_label) {
  label = new_label;
}

// Return ID + label combination
String MailBox::getName() const {
  String name;
  name += id;
  if (label != "") {
    name += F(" (");
    name += label;
    name += F(")");
  }
  return name;
}

// Return boot status (false/true == deep sleep/other)
bool MailBox::getBoot() const {
  return boot;
}

// Set boot status (false/true == deep sleep/other)
void MailBox::setBoot(const bool status) {
  boot = status;
}

// Return online status (false/true == offline/online)
bool MailBox::getOnline() const {
  return online;
}

// Set online status (false/true == offline/online)
void MailBox::setOnline(const bool status) {
  online = status;
}

// Return battery level (%)
uint8_t MailBox::getBattery() const {
  return battery;
}

// Set battery level (%)
void MailBox::setBattery(uint8_t level) {
  battery = level;
}

// Return door status (false/true == closed/open)
bool MailBox::getDoor() const {
  return door;
}

// Set door status (false/true == closed/open)
void MailBox::setDoor(bool status) {
  door = status;
}

// Return true if door is open
bool MailBox::doorOpen() const {
  return door;
}

// Return true if door is closed
bool MailBox::doorClosed() const {
  return !doorOpen();
}

// Return through message number
uint16_t MailBox::getMessageNumber() const {
  return msg_num;
}

// Set through message number
void MailBox::setMessageNumber(const uint16_t num) {
  msg_num = num;
}

// Return send messages counter
uint32_t MailBox::getMessageCount() const {
  return msg_count;
}

// Set sent messages counter
void MailBox::setMessageCount(const uint32_t num) {
  msg_count = num;
}

// Increment sent messages counter
//// We assume the counter does not overflow in any realistic scenario
void MailBox::incrementMessageCount(const uint32_t num) {
  msg_count += num;
}

// Comparison operator == (match by ID)
bool MailBox::operator==(const uint8_t id2) const {
  return id == id2;
}

// Comparison operator != (match by ID)
bool MailBox::operator!=(const uint8_t id2) const {
  return id != id2;
}

// Comparison operator < (match by ID)
bool MailBox::operator<(const uint8_t id2) const {
  return id < id2;
}

// Comparison operator > (match by ID)
bool MailBox::operator>(const uint8_t id2) const {
  return id > id2;
}

// Comparison operator <= (match by ID)
bool MailBox::operator<=(const uint8_t id2) const {
  return id <= id2;
}

// Comparison operator >= (match by ID)
bool MailBox::operator>=(const uint8_t id2) const {
  return id >= id2;
}

// Initialize message parts from mailbox data
MailBoxMessage& operator<<(MailBoxMessage& msg, const MailBox& mb) {
  msg.setMailBoxID(mb.getID());
  msg.setMessageNumber(mb.getMessageNumber());
  msg.setBoot(mb.getBoot());
  msg.setOnline(mb.getOnline());
  msg.setBattery(mb.getBattery());
  msg.setDoor(mb.getDoor());
  return msg;
}
