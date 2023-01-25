/* DS mailbox automation
 * * Local module
 * * * Google Assistant reporting implementation (via Assistant Relay, https://greghesp.github.io/assistant-relay/)
 * (c) DNS 2020-2023
 */

#include "MySystem.h"               // Syslog

#if defined(DS_SUPPORT_GOOGLE_ASSISTANT) && !defined(DS_MAILBOX_REMOTE)

#include "GoogleAssistant.h"

using namespace ds;

static const char *GA_CONF_FILE_NAME PROGMEM = "/google.cfg";

// Constructor
GoogleAssistant::GoogleAssistant(): active(false) {
}

// Return assistant relay location
const String& GoogleAssistant::getURL() const {
  return url;
}

// Set assistant relay location
void GoogleAssistant::setURL(const String& new_url) {
  url = new_url;
}

// Begin operations
void GoogleAssistant::begin() {

  // Load configuration if present
  load();
}

// Load configuration from disk
void GoogleAssistant::load() {
  auto file = System::fs.open(GA_CONF_FILE_NAME, "r");
  if (!file)
    return;
  url = file.readStringUntil('\r');
  active = file.parseInt();
  file.close();
  System::log->printf(TIMED("%s: Google Assistant Relay location: %s, %sactive\n"), GA_CONF_FILE_NAME, url.c_str(), active ? "" : "in");
}

// Save configuration to disk
void GoogleAssistant::save(const String& new_url, bool new_active) {
  url = new_url;
  active = new_active;
  auto file = System::fs.open(GA_CONF_FILE_NAME, "w");
  if (!file) {
    System::log->printf(TIMED("Error saving Google configuration\n"));
    return;
  }
  file.println(url);
  file.println(active ? 1 : 0);
  file.close();
}

// Activate service
void GoogleAssistant::activate() {
  active = true;
}

// Deactivate service
void GoogleAssistant::deactivate() {
  active = false;
}

// Return true if service is active
bool GoogleAssistant::isActive() const {
  return active;
}

// Broadcast message
bool GoogleAssistant::broadcast(const String& msg) {
  if (!active || !url.length())
    return false;

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

  return ret == HTTP_CODE_OK;
}

#endif // DS_SUPPORT_GOOGLE_ASSISTANT && !DS_MAILBOX_REMOTE
