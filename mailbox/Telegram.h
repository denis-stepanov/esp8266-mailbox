/* DS mailbox automation
 * * Local module
 * * * Telegram definition
 * (c) DNS 2020-2023
 */

#ifndef _DS_TELEGRAM_H_
#define _DS_TELEGRAM_H_

#include <WiFiClientSecure.h>        // SSL interface
#include <UniversalTelegramBot.h>    // Telegram bot library
#include "MySystem.h"                // Timers
#include "VirtualMailBox.h"          // Mailbox information

namespace ds {

  class Telegram {
      String token;                                   // Bot token
      String chat_id;                                 // Telegram chat ID
      WiFiClientSecure client;                        // Encrypted connection
      UniversalTelegramBot bot;                       // Bot instance
      TimerCountdownTick timer;                       // Timer to service Telegram incoming traffic
      bool active;                                    // True if service is active
      bool boot_reported;                             // True if boot has been already reported
      bool bounce_reported;                           // True if door bounce has been already reported
      bool update_in_progress;                        // True is update is in progress

      bool sendMessage(const String& /* chat_id */, const String& /* to */, const String& /* cmd */, const String& /* msg */); // Send message with logging
      void update();                                  // Process incoming commands

    public:
      Telegram();                                     // Constructor
      const String& getToken() const;                 // Return bot token
      void setToken(const String& /* new_token */);   // Set bot token
      const String& getChatID() const;                // Return chat ID
      void setChatID(const String& /* new_chat_id */);// Set chat ID
      void begin();                                   // Begin operations
      void load();                                    // Load configuration from disk
      void save(const String& /* new_token */, const String& /* new_chat_id */, bool /* new_active */); // Save configuration to disk
      void activate();                                // Activate service
      void deactivate();                              // Deactivate service
      bool isActive() const;                          // Return true if service is active
      bool sendTest(const String& /* new_token */, const String& /* new_chat_id */); // Send test message
      bool sendBoot();                                // Send boot notification
      bool sendBatteryLow(const VirtualMailBox& /* mb */); // Send low battery notification
      bool sendLostEvent(uint16_t /* num */);         // Send lost event notification
      bool sendEvent(const VirtualMailBox& /* mb */, uint16_t remote_time = 0); // Send event notification
  };

} // namespace ds

#endif // _DS_TELEGRAM_H_
