/* DS mailbox automation
 * * Local module
 * * * Receiver definition
 * (c) DNS 2020
 */

#ifndef _DS_RECEIVER_H_
#define _DS_RECEIVER_H_

#include "Transceiver.h"             // Transceiver
#include "MailBoxMessage.h"          // Mailbox message

namespace ds {

  class Receiver : public Transceiver {
      unsigned long t0;              // Time of first byte retrieval (ms from boot)
      bool recv_in_progress;         // Flag indicating that receiving is in progress
      uint8_t bytes_received;        // Number of bytes received

    public:
      Receiver(HardwareSerial &_serial = Serial, const uint8_t _tx_id = 0) :
        Transceiver(_serial, _tx_id), t0(0), recv_in_progress(false), bytes_received(0) {}
      void begin();                  // Receiver initialization
      void reset();                  // Reset pending transfer, if any
      void update();                 // Handle incoming traffic
      bool messageAvailable() const; // Check if message is available for the client
      MailBoxMessage getMessage(const bool and_reset = true); // Fetch the message, optionally deferring rearm for listening for a new message
  };

} // namespace ds

#endif // _DS_RECEIVER_H_
