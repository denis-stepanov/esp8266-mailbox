/* DS mailbox automation
 * * Communication protocol definition
 * (c) DNS 2020-2021
 */

#ifndef _DS_MAILBOXMESSAGE_H_
#define _DS_MAILBOXMESSAGE_H_

#include <Arduino.h>          // uint8_t, ...
#include <Printable.h>        // Printable class

/* Addressing scheme: <Communication Channel>.<Receiver ID>.<Mailbox ID>
 * * Communication Channel is pre-programmed into transmitter and receiver RF modules
 * * Receiver ID (1-15) - identifies receiver working on a given channel. 0 means message addressed to any receiver listening
 * * Mailbox ID (1-15) - identifies mailbox reporting to a given receiver. 0 is reserved for the receiver itself
 * E.g.: 07.01.02 - mailbox #2 reporting to receiver #1 on communication channel #7 (435.8 MHz)
 * * * * 15.03.00 - receiver #3 working on communication channel #15
 * Both receiver and transmitter RF modules must be pre-configured to use the same channel (see "rfconf" sketch)
 * Receiver must accept all messages carrying its ID + messages with receiver ID 0; other messages must be discarded (but may be logged)
 * Mailbox must identify itself with a non-zero ID
 * Each message carries a version of a protocol and a checksum; messages with mismatch in protocol or checksum must be discarded (but may be logged)
 * Protocol version 0 is reserved for development purposes. Applications may accept it at their own risk. Systems in production must use a non-zero version
 * Protocol version must be incremented every time an incompatible change is made
*/
namespace ds {

  // Protocol configuration
  const uint8_t PROTO_VERSION = 2;           // Protocol version (0-15)
  const uint8_t RECEIVER_ID_ANY = 0;         // Broadcast receiver address
  const uint8_t MESSAGE_NUMBER_UNKNOWN = 0;  // Unknown message number
  const uint8_t MAILBOX_ID_MIN = 1;          // Minimal mailbox ID (for use in probing)
  const uint8_t MAILBOX_ID_MAX = 15;         // Maximum mailbox ID (for use in probing)

  // Various battery levels (%)
  enum {
    BATTERY_LEVEL_DEAD /* = 0 */,            // Fully discharged
    BATTERY_LEVEL_LOW = 20,                  // Discharge alert
    BATTERY_LEVEL_FULL = 100,                // Fully charged
    BATTERY_LEVEL_UNKNOWN = 127              // Level unknown
  };

  // Transmission message (byte.bit). Byte order is network byte order
  typedef struct _mailbox_message {

    // 0: header
    uint8_t version : 4;    // 0.0-3 protocol version (0-15)
    uint8_t recv_id : 4;    // 0.4-7 receiver ID (0-15)

    // 1: mailbox status
    uint8_t mb_id : 4;      // 1.0-3: mailbox ID (1-15)
    uint8_t boot : 1;       // 1.4: boot status (0-wake up from deep sleep, 1-boot for other reason)
    uint8_t online : 1;     // 1.5: online status (0-offline (going to sleep), 1-online (staying awake))
    uint8_t door : 1;       // 1.6: door status (0-closed, 1-open)
    uint8_t _reserved : 1;  // 1.7: reserved

    // 2-3: through message number (1-65535; restarts at 1 on cold start. 0 is reserved as 'unknown')
    uint16_t msg_num;

    // 4-5: local time (ms from boot) (0-65535)
    uint16_t time;

    // 6: battery status
    uint8_t battery : 7;    // 6.0-6: 0-100%. 127 is reserved for 'unknown'
    uint8_t _reserved2 : 1; // 6.7: reserved

    // 7: checksum (always the last)
    uint8_t checksum;
  } __attribute__ ((packed)) mailbox_message;

  class MailBoxMessage : public Printable {
      union {
        mailbox_message msg;                         // Message as structure
        byte msg_buf[sizeof(msg)];                   // Message as buffer
      };
      bool print_raw;                                // True if messages should be printed as raw buffer instead of human-readable string

    protected:
      uint8_t checksum() const;                      // Calculate checksum

    public:
      void init(const uint8_t /* rx_id */);          // Initialize the message
      void terminate();                              // Finalize the message
      bool protocolVersionOK() const;                // Check protocol version
      bool checksumOK() const;                       // Verify checksum
      size_t getSize() const;                        // Return message size (B)
      void setByte(const uint8_t /* pos */, const byte /* value */); // Set individual byte in the message buffer
      byte& operator[](const uint8_t /* pos */);     // Set individual byte in the message buffer
      uint8_t getReceiverID() const;                 // Return receiver ID
      uint8_t getProtocolVersion() const;            // Return protocol version
      uint16_t getMessageNumber() const;             // Return through message number
      void setMessageNumber(const uint16_t /* num */); // Set through message number
      static uint16_t getNextMessageNumber(uint16_t& /* num */); // Get next message number (overflow-safe)
      uint16_t getTime() const;                      // Return time (ms since boot)
      void setTime(const unsigned long /* ms */);    // Set time (ms since boot)
      uint8_t getBattery() const;                    // Return battery level (%)
      void setBattery(const uint8_t /* level */);    // Set battery level (%)
      uint8_t getMailBoxID() const;                  // Return mailbox ID
      void setMailBoxID(const uint8_t /* id */);     // Set mailbox ID
      bool getBoot() const;                          // Return boot status (false/true == deep sleep/other)
      void setBoot(const bool /* status */);         // Set boot status (false/true == deep sleep/other)
      bool getOnline() const;                        // Return online status (false/true == offline/online)
      void setOnline(const bool /* status */);       // Set online status (false/true == offline/online)
      bool getDoor() const;                          // Return door status (false/true == closed/open)
      void setDoor(const bool /* status */);         // Set door status (false/true == closed/open)
      const MailBoxMessage& asIs();                  // Switch printing preference to default (parsed)
      const MailBoxMessage& asRaw();                 // Switch printing preference to raw
      size_t printTo(Print& /* log */) const;        // Print message into a log
      size_t send(Stream& /* tx */) const;           // Send message
  };

} // namespace ds

#endif // _DS_MAILBOXMESSAGE_H_
