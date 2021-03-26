/* DS mailbox automation
 * * Local module
 * * * Mailbox Manager definition
 * (c) DNS 2020-2021
 */

#ifndef _DS_MAILBOXMANAGER_H_
#define _DS_MAILBOXMANAGER_H_

#include <forward_list>       // Linked list
#include <Arduino.h>          // uint8_t, ...
#include <Ticker.h>           // Periodic events
#include "VirtualMailBox.h"   // Mailbox definition
#include "MailBoxMessage.h"   // Mailbox message

namespace ds {
  
  // Collection of mailboxes
  class MailBoxManager {
      std::forward_list<VirtualMailBox *> mailboxes;  // List of mailboxes served by this module
      mailbox_alarm alarm;                            // Global alarm level
      Ticker ticker;                                  // Timer to check for absent mailboxes

    public:
      MailBoxManager();                               // Constructor
      ~MailBoxManager();                              // Collection destructor (normally never called)
      void begin();                                   // Initialize mailboxes
      VirtualMailBox *getMailBox(const uint8_t /* mb_id */, bool create = false); // Find mailbox by ID. If not found, allow registering a new one
      VirtualMailBox *operator[](const uint8_t /* mb_id */); // Find existing mailbox by ID
      bool update(const MailBoxMessage& /* msg */);   // Update mailbox from received message; create if not found
      bool deleteMailBox(const uint8_t /* mb_id */);  // Delete mailbox with a given ID
      void updateAlarm();                             // Update global alarm and its display with the latest status from mailboxes
      mailbox_alarm acknowledgeAlarm(const String& /* via */); // Acknowledge global alarm. Returns the alarm acknowledged
      void rollCall();                                // Regular check of mailboxes' status
      void printHTML(String& /* buf */);              // Print mailboxes table in HTML
      void printText(String& /* buf */);              // Print mailboxes table in text
  };

} // namespace ds

String& operator<<(String& /* html_buf */, ds::MailBoxManager& /* mbm */);  // HTML printout helper

#endif // _DS_MAILBOXMANAGER_H_
