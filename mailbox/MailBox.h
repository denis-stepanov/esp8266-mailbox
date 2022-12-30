/* DS mailbox automation
 * * Basic mailbox definition
 * (c) DNS 2020-2022
 */

#ifndef _DS_MAILBOX_H_
#define _DS_MAILBOX_H_

#include <Arduino.h>         // uint8_t, ...
#include "MailBoxMessage.h"  // MESSAGE_NUMBER_UNKNOWN

namespace ds {

  // Basic mailbox class
  class MailBox {

    protected:
      const uint8_t id;                      // Mailbox id
      String label;                          // Mailbox label
      bool boot;                             // Boot status (false/true == deep sleep/cold boot)
      bool online;                           // Online status (false/true == going offline/staying online)
      uint8_t battery;                       // Battery level (%)
      bool door;                             // Door status (false/true == closed/open)
      uint16_t msg_num;                      // Through message number
      uint32_t msg_count;                    // Sent messages counter

    public:
      const unsigned int AWAKE_TIME = 30000; // Max time for remote module to stay awake after the door was open (ms)

      MailBox(const uint8_t _id = 1, const String _label = (char *)nullptr, const uint8_t _battery = BATTERY_LEVEL_UNKNOWN) :
        id(_id), label(_label), boot(false), online(false), battery(_battery), door(false), msg_num(MESSAGE_NUMBER_UNKNOWN), msg_count(0) {}

      uint8_t getID() const;                 // Return mailbox ID
      const String& getLabel() const;        // Return mailbox label
      void setLabel(const String& /* new_label */); // Set mailbox label
      String getName() const;                // Return ID + label combination
      bool getBoot() const;                  // Return boot status (false/true == deep sleep/other)
      void setBoot(const bool /* status */); // Set boot status (false/true == deep sleep/other)
      bool getOnline() const;                // Return online status (false/true == offline/online)
      void setOnline(const bool /* status */); // Set online status (false/true == offline/online)
      uint8_t getBattery() const;            // Return battery level (%)
      void setBattery(uint8_t /* level */);  // Set battery level (%)
      bool getDoor() const;                  // Return door status (false/true == closed/open)
      void setDoor(bool /* status */);       // Set door status (false/true == closed/open)
      bool doorOpen() const;                 // Return true if door is open
      bool doorClosed() const;               // Return true if door is closed
      uint16_t getMessageNumber() const;     // Return through message number
      void setMessageNumber(const uint16_t /* num */); // Set through message number
      uint32_t getMessageCount() const;      // Return sent messages counter
      void setMessageCount(const uint32_t num = 0); // Set sent messages counter
      void incrementMessageCount(const uint32_t num = 1); // Increment sent messages counter

      bool operator==(const uint8_t /* id2 */) const; // Comparison operator == (match by ID)
      bool operator!=(const uint8_t /* id2 */) const; // Comparison operator != (match by ID)
      bool operator<(const uint8_t /* id2 */) const;  // Comparison operator <  (match by ID)
      bool operator>(const uint8_t /* id2 */) const;  // Comparison operator >  (match by ID)
      bool operator<=(const uint8_t /* id2 */) const; // Comparison operator <= (match by ID)
      bool operator>=(const uint8_t /* id2 */) const; // Comparison operator >= (match by ID)
      bool operator==(const MailBox& /* mb */) const; // Comparison operator == (match by ID)
      bool operator!=(const MailBox& /* mb */) const; // Comparison operator != (match by ID)
      bool operator<(const MailBox& /* mb */) const;  // Comparison operator <  (match by ID)
      bool operator>(const MailBox& /* mb */) const;  // Comparison operator >  (match by ID)
      bool operator<=(const MailBox& /* mb */) const; // Comparison operator <= (match by ID)
      bool operator>=(const MailBox& /* mb */) const; // Comparison operator >= (match by ID)
  };

} // namespace ds

ds::MailBoxMessage& operator<<(ds::MailBoxMessage& /* msg */, const ds::MailBox& /* mb */);  // Initialize message parts from mailbox data

#endif // _DS_MAILBOX_H_
