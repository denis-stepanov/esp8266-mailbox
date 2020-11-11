/* DS mailbox automation
 * * Remote module
 * * * Mailbox definition
 * (c) DNS 2020
 */

#ifndef _DS_PHYSICALMAILBOX_H_
#define _DS_PHYSICALMAILBOX_H_

#include "MailBox.h"               // Base class
#include "MailBoxMessage.h"        // Mailbox message

namespace ds {

  // Physical mailbox class
  class PhysicalMailBox : public MailBox {

    protected:
      const int pin_door;          // Door sensor pin
    
    public:
      PhysicalMailBox(const uint8_t _id, const int _pin_door) :
        MailBox(_id), pin_door(_pin_door) {}
      void begin();                // Initialize mailbox
      void update();               // Update mailbox status
      uint8_t updateBattery();     // Update and return battery level (%)
      void sleep() const;          // Put mailbox to sleep
  };

} // namespace ds

#endif // _DS_PHYSICALMAILBOX_H_
