/* DS mailbox automation
 * * Local module
 * * * Telegram definition
 * (c) DNS 2020-2021
 */

#ifndef _DS_TELEGRAM_H_
#define _DS_TELEGRAM_H_

#include <WiFiClientSecure.h>        // SSL interface
#include <UniversalTelegramBot.h>    // Telegram bot library
#include "MySystem.h"                // Timers
#include "VirtualMailBox.h"          // Mailbox information

namespace ds {

  class Telegram {
      static const char *token;                       // Bot token
      static const char *chat_id;                     // Telegram chat id
      WiFiClientSecure client;                        // Encrypted connection
      UniversalTelegramBot bot;                       // Bot instance
      TimerCountdownTick timer;                       // Timer to service Telegram incoming traffic
      bool boot_reported;                             // True if boot has been already reported
      bool bounce_reported;                           // True if door bounce has been already reported

      bool sendMessage(const String& /* chat_id */, const String& /* to */, const String& /* cmd */, const String& /* msg */); // Send message with logging
      void update();                                  // Process incoming commands

    public:
      Telegram();                                     // Constructor
      bool sendBoot();                                // Send boot notification
      bool sendLostEvent(uint16_t /* num */);         // Send lost event notification
      bool sendEvent(const VirtualMailBox& /* mb */, uint16_t remote_time = 0); // Send event notification
  };

} // namespace ds

#endif // _DS_TELEGRAM_H_
