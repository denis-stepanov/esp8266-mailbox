/* DS mailbox automation
 * * Local module
 * * * Receiver implementation
 * (c) DNS 2020-2022
 */

#include "MySystem.h"       // System log

#ifndef DS_MAILBOX_REMOTE

#include "Receiver.h"
#include <StreamString.h>   // Streamed string

using namespace ds;

#ifdef DS_DEVBOARD
// We have no hardware receiver on dev board, so emulate the incoming message
bool recv_message_emulated = false;              // Flag to emulate message arrival
static MailBoxMessage msg_emulated;              // Emulated message
#endif //DS_DEVBOARD

// Receiver initialization
void Receiver::begin() {

  System::log->printf(TIMED("Initializing RF receiver... "));

#ifdef DS_DEVBOARD
  msg_emulated.init(RECEIVER_ID);
  msg_emulated.setMailBoxID(1);
#else
  // Configure RX line only
  serial.begin(RF_SPEED, SERIAL_8N1, SERIAL_RX_ONLY);
#endif // DS_DEVBOARD
  reset();
  System::log->println("OK");
}

// Reset pending transfer, if any
void Receiver::reset() {
  t0 = millis();
  recv_in_progress = false;
  bytes_received = 0;
}

// Handle incoming traffic
void Receiver::update() {
  StreamString lmsg;

#ifdef DS_DEVBOARD
  if (recv_message_emulated) {
    recv_message_emulated = false;

    // Sample message
    msg_emulated.setMessageNumber(msg_emulated.getMessageNumber() + 1);
    msg_emulated.setDoor(msg_emulated.getMessageNumber() % 2);        // Flip the door every time
    msg_emulated.setTime(msg_emulated.getDoor() ? 200 : 5000);        // Typical time sequence
    msg_emulated.setBattery(50 /* base == 50% */ + millis() % 10 - 5  /* randomize +- 5% */);
    msg_emulated.setBoot(msg_emulated.getMessageNumber() <= 2);       // First two messages are boot messages
    msg_emulated.setOnline(msg_emulated.getDoor());                   // Set online on open, offline on close
    msg_emulated.terminate();
    msg = msg_emulated;
    recv_in_progress = false;
    bytes_received = msg.getSize();

    lmsg = F("Received message: ");
    lmsg.print(msg.asIs());
    System::log->printf(TIMED("%s\n"), lmsg.c_str());
  }
#else
  while (serial.available() > 0 && bytes_received < msg.getSize()) {

    if (!recv_in_progress) {
      reset();
      recv_in_progress = true;
    }
    const auto b = serial.read();      // Read 1 byte
    if (b != -1)
      msg[bytes_received++] = b;
    else {
      lmsg = F("Error reading input message after ");
      lmsg += bytes_received;
      lmsg += F(" byte(s)");
      System::appLogWriteLn(lmsg, true);
      break;
    }

    // Check integrity
    if (bytes_received == 1 && (!msg.protocolVersionOK() || !receiverIDOK())) {
      if (!msg.protocolVersionOK()) {
        lmsg = F("Invalid message: wrong protocol version: ");
        lmsg += msg.getProtocolVersion();
        lmsg += F(" (expected ");
        lmsg += PROTO_VERSION;
        lmsg += F("), ignoring");
      } else {
        lmsg = F("Invalid message: wrong receiver: ");
        lmsg += msg.getReceiverID();
        lmsg += F(" (expected ");
        lmsg += RECEIVER_ID;
        lmsg += F("), ignoring");
      }
      System::appLogWriteLn(lmsg, true);
      reset();
    }
  }

  if (recv_in_progress && bytes_received == msg.getSize()) {
    recv_in_progress = false;
    if (msg.checksumOK()) {
      lmsg = F("Received message: ");
      lmsg.print(msg.asIs());
      System::log->printf(TIMED("%s\n"), lmsg.c_str());
    } else {
      lmsg = F("Invalid message: checksum mismatch; raw=");
      lmsg.print(msg.asRaw());
      reset();
      System::appLogWriteLn(lmsg, true);
    }
  }

  if (recv_in_progress && millis() - t0 > RF_TIMEOUT) {
    lmsg = F("Message timeout after receiving ");
    lmsg += bytes_received;
    lmsg += F("/");
    lmsg += msg.getSize();
    lmsg += F(" bytes");
    System::appLogWriteLn(lmsg, true);
    reset();
  }
#endif // DS_DEVBOARD
}

// Check if message is available for the client
bool Receiver::messageAvailable() const {
  return !recv_in_progress && bytes_received == msg.getSize();
}

// Fetch the message, optionally deferring rearm for listening for a new message
MailBoxMessage Receiver::getMessage(const bool and_reset) {
  if (and_reset)
    reset();
  return msg;
}

#endif // !DS_MAILBOX_REMOTE
