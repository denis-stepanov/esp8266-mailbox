/* DS mailbox automation
 * * Remote module
 * * * Transmitter implementation
 * * * Hardcoded delays largely depend on communication mode selected in "rfconf" sketch
 * (c) DNS 2020
 */

#include "MySystem.h"         // System log

#ifdef DS_MAILBOX_REMOTE

#include "Transmitter.h"

using namespace ds;

// Initialize transmitter
void Transmitter::begin() {
  System::log->printf(TIMED("Initializing RF transmitter... "));

  // Set up I/O
  pinMode(pin_set, OUTPUT);
  digitalWrite(pin_set, HIGH);

  // Configure TX line only
  serial.begin(RF_SPEED, SERIAL_8N1, SERIAL_TX_ONLY);
  System::log->println("OK");
}

// Put transmitter to sleep mode
void Transmitter::sleep() const {
  System::log->printf(TIMED("Putting transmitter to sleep\n"));
  digitalWrite(pin_set, LOW);
  delay(250);
  serial.println("AT+SLEEP");
  delay(250);
  digitalWrite(pin_set, HIGH);
}

// Wake the transmitter up
void Transmitter::wakeup() const {
  System::log->printf(TIMED("Waking transmitter up\n"));
  digitalWrite(pin_set, LOW);
  delay(10);
  digitalWrite(pin_set, HIGH);
  delay(400);
}

// Send mailbox status
void Transmitter::send(MailBox &mb) {

  // Prepare
  auto msg_num = mb.getMessageNumber();
  mb.setMessageNumber(msg.getNextMessageNumber(msg_num));
  mb.incrementMessageCount();
  msg.init(RECEIVER_ID);
  msg << mb;
  msg.setTime(millis());
  msg.terminate();

  // Send
  msg.send(serial);
  System::log->printf(TIMED("Sending "));
  System::log->print("message: ");
  System::log->print(msg.asIs());
  System::log->print("; raw=");
  System::log->print(msg.asRaw());
  System::log->println();
  delay(1500);   // Make sure this is fully transmitted, before doing anything else
}

// Operator to send maibox status
Transmitter& operator<<(Transmitter& t, MailBox& mb) {
  t.send(mb);
  return t;
}

#endif // DS_MAILBOX_REMOTE
