/* DS mailbox automation
 * * Local module
 * * * Google Assistant reporting definition
 */

#ifndef _DS_GOOGLEASSISTANT_H_
#define _DS_GOOGLEASSISTANT_H_

#include <ESP8266HTTPClient.h>             // HTTP interface

namespace ds {

  class GoogleAssistant {
      static const char *url;              // Assistant relay location
      WiFiClient client;                   // WiFi interface
      HTTPClient http;                     // HTTP interface

    public:
      bool broadcast(const String& msg);   // Broadcast message
  };

} // namespace ds

#endif // _DS_GOOGLEASSISTANT_H_
