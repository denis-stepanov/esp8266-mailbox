/* DS mailbox automation
 * * Local module
 * * * Google Assistant reporting implementation (via Assistant Relay, https://greghesp.github.io/assistant-relay/)
 * (c) DNS 2020
 */

#include "MySystem.h"               // Syslog

#if defined(DS_SUPPORT_GOOGLE_ASSISTANT) && !defined(DS_MAILBOX_REMOTE)

#include "GoogleAssistant.h"

using namespace ds;

// Assistant relay location. Specify where your relay is deployed
const char *GoogleAssistant::url PROGMEM = "http://192.168.1.2:3000/assistant";

// Broadcast message
bool GoogleAssistant::broadcast(__attribute__ ((unused)) const String& msg) {

#ifdef DS_DEVBOARD
  const int ret = HTTP_CODE_OK;    // Skip assistant announcements
#else
  http.begin(client, url);
  http.addHeader(F("Content-Type"), F("application/json"));
  String payload;
  payload += F("{\"command\": \"");
  payload += msg;          // msg must not contain quotes
  payload += F("\", \"broadcast\": true}");
  System::log->printf(TIMED("Sending broadcast to Google Assistant... "));
  const auto ret = http.POST(payload);
  System::log->println(ret ? F("OK") : F("failed"));
  http.end();
#endif // DS_DEVBOARD

  return ret == HTTP_CODE_OK;
}

#endif // DS_SUPPORT_GOOGLE_ASSISTANT && !DS_MAILBOX_REMOTE
