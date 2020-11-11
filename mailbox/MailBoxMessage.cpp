/* DS mailbox automation
 * * Communication protocol implementation
 * (c) DNS 2020
 */

#include "MailBoxMessage.h"
#include <lwip/inet.h>        // htons() / ntohs()

using namespace ds;

// Calculate checksum
uint8_t MailBoxMessage::checksum() const {
  uint8_t sum = 0;
  for (uint8_t i = 0; i < sizeof(msg_buf) - 1; i++)
    sum ^= msg_buf[i];
  return sum;
}

// Initialize the message
void MailBoxMessage::init(const uint8_t rx_id) {
  memset(msg_buf, 0, sizeof(msg_buf));
  msg.version = PROTO_VERSION;
  msg.recv_id = rx_id;
}

// Finalize the message
void MailBoxMessage::terminate() {
  msg.checksum = checksum();
}

// Check protocol version
bool MailBoxMessage::protocolVersionOK() const {
  return msg.version == PROTO_VERSION;
}

// Verify checksum
bool MailBoxMessage::checksumOK() const {
  return msg.checksum == checksum();
}

// Return message size (B)
size_t MailBoxMessage::getSize() const {
  return sizeof(msg_buf);
}

// Set individual byte in the message buffer
void MailBoxMessage::setByte(const uint8_t pos, const byte value) {
  if (pos < sizeof(msg_buf))
    msg_buf[pos] = value;
}

// Set individual byte in the message buffer
byte& MailBoxMessage::operator[](const uint8_t pos) {

  // Returning first byte on out of bounds is not entirely correct, but it is better than returning some wild address
  return msg_buf[pos < sizeof(msg_buf) ? pos : 0];
}

// Return receiver ID
uint8_t MailBoxMessage::getReceiverID() const {
  return msg.recv_id;
}

// Return protocol version
uint8_t MailBoxMessage::getProtocolVersion() const {
  return msg.version;
}

// Return through message number
uint16_t MailBoxMessage::getMessageNumber() const {
  return ntohs(msg.msg_num);
}

// Set through message number
void MailBoxMessage::setMessageNumber(const uint16_t num) {
  msg.msg_num = htons(num);
}

// Get next message number (overflow-safe)
uint16_t MailBoxMessage::getNextMessageNumber(uint16_t& num) {
  if (++num == MESSAGE_NUMBER_UNKNOWN)
    num += 2;    // Avoid sending 0 on overflow. Also skip one more number to keep numbering symmetric (odd=start, even=sleep)
  return num;
}

// Return time (ms since boot)
uint16_t MailBoxMessage::getTime() const {
  return ntohs(msg.time);
}

// Set time (ms since boot)
void MailBoxMessage::setTime(const unsigned long ms) {
  msg.time = htons(ms);
}

// Return battery level (%)
uint8_t MailBoxMessage::getBattery() const {
  return msg.battery;
}

// Set battery level (%)
void MailBoxMessage::setBattery(const uint8_t level) {
  msg.battery = level;
}

// Return mailbox ID
uint8_t MailBoxMessage::getMailBoxID() const {
  return msg.mb_id;
}

// Set mailbox ID
void MailBoxMessage::setMailBoxID(const uint8_t id) {
  msg.mb_id = id;
}

// Return boot status (false/true == deep sleep/other)
bool MailBoxMessage::getBoot() const {
  return msg.boot;
}

// Set boot status (false/true == deep sleep/other)
void MailBoxMessage::setBoot(const bool status) {
  msg.boot = status;
}

// Return online status (false/true == offline/online)
bool MailBoxMessage::getOnline() const {
  return msg.online;
}

// Set online status (false/true == offline/online)
void MailBoxMessage::setOnline(const bool status) {
  msg.online = status;
}

// Return door status (false/true == closed/open)
bool MailBoxMessage::getDoor() const {
  return msg.door;
}

// Set door status (false/true == closed/open)
void MailBoxMessage::setDoor(const bool status) {
  msg.door = status;
}

// Switch printing preference to default (parsed)
const MailBoxMessage& MailBoxMessage::asIs() {
  print_raw = false;
  return *this;
}

// Switch printing preference to raw
const MailBoxMessage& MailBoxMessage::asRaw() {
  print_raw = true;
  return *this;
}

// Print message into a log
size_t MailBoxMessage::printTo(Print& log) const {
  size_t printed = 0;
  if (print_raw)
    for (size_t i = 0; i < sizeof(msg_buf); i++) {
      printed += log.printf("%02x", msg_buf[i]);
      if (i < sizeof(msg_buf) - 1)
        printed += log.print(".");
    }
  else
    printed += log.printf("mailbox=%hhu, msgnum=%hu, time=%hu, battery=%hhu, coldboot=%s, online=%s, door=%s", getMailBoxID(),getMessageNumber(), getTime(),
      getBattery(), getBoot() ? "yes" : "no", getOnline() ? "yes" : "no", getDoor() ? "open" : "closed");
  return printed;
}

// Send message
size_t MailBoxMessage::send(Stream& tx) const {
  return tx.write(msg_buf, sizeof(msg_buf));
}
