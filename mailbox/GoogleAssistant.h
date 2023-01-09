/* DS mailbox automation
 * * Local module
 * * * Google Assistant reporting definition
 * (c) DNS 2020-2023
 */

#ifndef _DS_GOOGLEASSISTANT_H_
#define _DS_GOOGLEASSISTANT_H_

#include <ESP8266HTTPClient.h>             // HTTP interface

namespace ds {

  class GoogleAssistant {
      String url;                          // Assistant relay location
      WiFiClient client;                   // WiFi interface
      HTTPClient http;                     // HTTP interface

    public:
      const String& getURL() const;        // Return assistant relay location
      void setURL(const String& /* new_url */);     // Set assistant relay location
      void begin();                        // Begin operations
      void load();                         // Load configuration from disk
      void save(const String& /* new_url */);       // Save configuration to disk
      bool broadcast(const String& /* msg */);      // Broadcast message
  };

} // namespace ds

#endif // _DS_GOOGLEASSISTANT_H_
