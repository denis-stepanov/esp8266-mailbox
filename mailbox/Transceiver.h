/* DS mailbox automation
 * * Transceiver definition
 * (c) DNS 2020
 */

#ifndef _DS_TRANSCEIVER_H_
#define _DS_TRANSCEIVER_H_

#include <Arduino.h>                 // uint8_t, ...
#include <HardwareSerial.h>          // HardwareSerial class
#include "MailBoxMessage.h"          // Protocol definition

namespace ds {

  class Transceiver {
    
    protected:
      const uint8_t RECEIVER_ID = 1;             // Receiver ID
      const unsigned int RF_SPEED = 1200;        // RF comminucation speed (bod). Depends on communication mode selected in "rfconf" sketch
      const unsigned int RF_TIMEOUT = 2000;      // Message receiving timeout (ms). Depends on communication mode chosen in "rfconf" sketch

      HardwareSerial &serial;                    // Communication interface
      const uint8_t tx_id;                       // Transmitter ID (0 for any)
      MailBoxMessage msg;                        // Exchange message

      bool receiverIDOK() const;                 // Check receiver ID

    public:
      Transceiver(HardwareSerial &_serial = Serial, const uint8_t _tx_id = 1) :
        serial(_serial), tx_id(_tx_id) {}
      virtual void begin() {}                    // Transceiver initialization
  };

} // namespace ds

#endif // _DS_TRANSCEIVER_H_
