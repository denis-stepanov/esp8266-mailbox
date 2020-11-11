/* DS mailbox automation
 * * Remote module
 * * * Transmitter definition
 * (c) DNS 2020
 */

#ifndef _DS_TRANSMITTER_H_
#define _DS_TRANSMITTER_H_

#include "Transceiver.h"             // Transceiver
#include "MailBox.h"                 // Mailbox

namespace ds {
  
  class Transmitter : public Transceiver {
      const int pin_set;                   // Transmitter control pin

    public:
      Transmitter(HardwareSerial &_serial = Serial, const uint8_t _tx_id = 1, const int _pin_set = 0) :
        Transceiver(_serial, _tx_id), pin_set(_pin_set) {}
      void begin();                        // Initialize transmitter
      void sleep() const;                  // Put transmitter to sleep mode
      void wakeup() const;                 // Wake the transmitter up
      void send(MailBox& /* mb */);        // Send mailbox status
  }; 

} // namespace ds

ds::Transmitter& operator<<(ds::Transmitter& /* t */, ds::MailBox& /* mb */);  // Operator to send maibox status

#endif // _DS_TRANSMITTER_H_
