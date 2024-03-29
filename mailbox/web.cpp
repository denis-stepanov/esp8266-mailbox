/* DS mailbox automation
 * * Local module
 * * * Web pages implementation
 * (c) DNS 2020-2023
 */

#include "MySystem.h"               // System-level definitions

#ifndef DS_MAILBOX_REMOTE

#include "MailBoxManager.h"         // Mailbox manager
#include "GoogleAssistant.h"        // Google interface
#ifdef DS_SUPPORT_TELEGRAM
#include "Telegram.h"               // Telegram interface
#endif // DS_SUPPORT_TELEGRAM

using namespace ds;

// Server data providers
extern MailBoxManager mailbox_manager;     // Mailbox manager instance
extern GoogleAssistant google_assistant;   // Google interface
#ifdef DS_SUPPORT_TELEGRAM
extern Telegram telegram;                  // Telegram interface
#endif // DS_SUPPORT_TELEGRAM

// Initialize page buffer with page header
static void pushHeader(const String& title, bool redirect = false) {
  String &page = System::web_page;
  String uri = System::web_server.uri();

  // UTF-8 'OPEN MAILBOX WITH RAISED FLAG'
  String head_user = F(
    "<link rel=\"icon\" href='data:image/svg+xml;utf8,<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 240 240\"><text y=\"192\" font-size=\"192\">&#x1f4ec;</text></svg>'/>\n"
  );
  head_user += uri == "/" ?
    // Note: this must match the ALARM_* enum
    F(
    "<style>\n"
    "  .alarm  { color: red; }\n"
    "  .alarm0 { }\n"
    "  .alarm1 { }\n"
    "  .alarm2 { }\n"
    "  .alarm3 { }\n"
    "  .alarm4 { font-weight: bold; }\n"
    "  .alarm5 { font-weight: bold; color: red; }\n"
    "  .alarm6 { font-weight: bold; color: red; animation: blinker 0.6s linear infinite; }\n"
    "  @keyframes blinker { 50% { opacity: 0; } }\n"
    "</style>\n") : F("");

  System::pushHTMLHeader(title, head_user, redirect);
  page += F("<h3>");
  page += title;
  page += F("</h3>\n");

  // Navigation
  page += F(
    "<table cellpadding=\"0\" cellspacing=\"0\" width=\"100%\"><tr><td>"
    "[&nbsp;<a href=\"/\">home</a>&nbsp;] "
    "[&nbsp;<a href=\"/conf\">conf</a>&nbsp;] "
    "[&nbsp;<a href=\"/log\">log</a>&nbsp;] "
    "[&nbsp;<a href=\"/about\">about</a>&nbsp;]"
    "</td><td align=\"right\">"
  );
  if (System::getTimeSyncStatus() != TIME_SYNC_NONE)
    page += System::getTimeStr();
  page += F(
    "</td></tr></table>"
    "<hr/>\n"
  );
}

// Append page footer to the buffer
static void pushFooter() {
  System::pushHTMLFooter();
}

// Serve the root page
static void serveRoot() {
  pushHeader(F("Mailbox Manager"));
  System::web_page += "<center>\n";
  System::web_page << mailbox_manager;
  System::web_page += "</center>\n";
  pushFooter();
  System::sendWebPage();
}

// Serve the mailbox page
static void serveMailBox() {
  if (System::web_server.args() == 1) {
    const uint8_t id = System::web_server.arg(0).toInt();
    if (id) {
      const auto mailbox = mailbox_manager[id];
      if (mailbox) {
        pushHeader((String)"Mailbox " + mailbox->getName());
        String &page = System::web_page;
        page += F("<form action=\"/save\">\n"
                  "<input type=\"hidden\" name=\"id\" value=\"");
        page += id;
        page += F("\">\n"
                  "<p><label for=\"label\">Configure label: </label>"
                  "<input type=\"text\" id=\"label\" name=\"label\" value=\"");
        page += mailbox->getLabel();
        page += F("\"/></p>\n"
                  "<p><button type=\"submit\" name=\"action\" value=\"save\">Save</button> "
                  "<button type=\"submit\" name=\"action\" value=\"del\"/>Forget Mailbox</button></p>\n"
                  "</form>\n");
      } else
        pushHeader(F("Mailbox Not Found"));
    } else
      pushHeader(F("Invalid Parameters"));
  } else
    pushHeader(F("Invalid Parameters"));
  pushFooter();
  System::sendWebPage();
}

// Serve the mailbox configuration saving page
static void serveSave() {
  if (System::web_server.args() == 3) {
    uint8_t id = 0;
    String label, action;
    auto id_ok = false, label_ok = false, action_ok = false;
    for (unsigned int i = 0; i < (unsigned int)System::web_server.args(); i++) {
      String arg_name = System::web_server.argName(i);
      if (arg_name == "id") {
        id = System::web_server.arg(i).toInt();
        id_ok = true;
      } else
      if (arg_name == "action") {
        action = System::web_server.arg(i);
        action_ok = true;
      } else
      if (arg_name == "label") {
        label = System::web_server.arg(i);
        label_ok = true;
      }
    }
    if (id_ok && action_ok && label_ok) {
      if (action == "save") {
        const auto mailbox = mailbox_manager[id];
        if (mailbox) {
          String old_label = mailbox->getLabel();
          mailbox->setLabel(label);
          mailbox->save();
          pushHeader(F("Configuration Saved"), true);

          String lmsg = F("Mailbox ");
          lmsg += id;
          lmsg += F(" label updated from \"");
          lmsg += old_label;
          lmsg += F("\" to \"");
          lmsg += label;
          lmsg += F("\" from ");
          lmsg += System::web_server.client().remoteIP().toString();
          System::appLogWriteLn(lmsg);
        } else
          pushHeader(F("Mailbox Not Found"), true);
      } else
        if (action == "del") {
          pushHeader(mailbox_manager.deleteMailBox(id) ? F("Mailbox Forgotten") : F("Mailbox Not Found"), true);

          String lmsg = F("Mailbox ");
          lmsg += id;
          lmsg += F(" settings erased from ");
          lmsg += System::web_server.client().remoteIP().toString();
          System::appLogWriteLn(lmsg);
        } else
          pushHeader(F("Unknown action"), true);
    } else
      pushHeader(F("Invalid Parameters"), true);
  } else
    pushHeader(F("Invalid Parameters"), true);
  pushFooter();
  System::sendWebPage();
}

// Acknowledge global alarm
static void serveAcknowledge() {
  uint8_t id = 0;
  String via = F("web from ");
  via += System::web_server.client().remoteIP().toString();

  for (unsigned int i = 0; i < (unsigned int)System::web_server.args(); i++) {
    String arg_name = System::web_server.argName(i);
    if (arg_name == "id")
      id = System::web_server.arg(i).toInt();
  }

  mailbox_manager.acknowledgeAlarm(via, id);
  pushHeader(F("Alarm Acknowledged"), true);
  pushFooter();
  System::sendWebPage();
}

// Serve the global configuration page
static void serveConf() {
  pushHeader(F("Global Configuration"));
  String &page = System::web_page;
  page += F("<form action=\"/confSave\">\n");

  page += F(
    "  <p>\n"
    "    <input name=\"g_active\" type=\"checkbox\"");
  if (google_assistant.isActive())
    page += F(" checked=\"checked\"");
  page += F("/> Google Assistant:<br/>\n"
    "    &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<label for=\"g_url\">Relay URL (<i>http://IP:PORT/assistant</i>): </label>\n"
    "    <input type=\"text\" size=\"40\" id=\"g_url\" name=\"g_url\" value=\"");
  page += google_assistant.getURL();
  page += F("\"/>\n"
    "    <button type=\"submit\" name=\"action\" value=\"test_google\">Test</button>\n"
    "  </p>\n");

#ifdef DS_SUPPORT_TELEGRAM
  page += F(
    "  <p>\n"
    "    <input name=\"t_active\" type=\"checkbox\"");
  if (telegram.isActive())
    page += F(" checked=\"checked\"");
  page += F("/> Telegram:<br/>\n"
    "    &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<label for=\"t_token\">Bot Token:</label>\n"
    "    <input type=\"text\" size=\"40\" id=\"t_token\" name=\"t_token\" value=\"");
  page += telegram.getToken();
  page += F("\"/><br/>\n"
    "    &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<label for=\"t_chat_id\">Chat ID:&nbsp;&nbsp;&nbsp;&nbsp;</label>\n"
    "    <input type=\"text\" size=\"40\" id=\"t_chat_id\" name=\"t_chat_id\" value=\"");
  page += telegram.getChatID();
  page += F("\"/>\n"
    "    <button type=\"submit\" name=\"action\" value=\"test_telegram\">Test</button>\n"
    "  </p>\n");
#endif // DS_SUPPORT_TELEGRAM

  page += F("  <p><button type=\"submit\" name=\"action\" value=\"save\">Save</button></p>");
  page += F("</form>\n");
  pushFooter();
  System::sendWebPage();
}

// Serve the global configuration saving page
static void serveConfSave() {

  String action;
  auto action_ok = false;

  String g_url;
  auto g_url_ok = false;
  auto g_active = false;

#ifdef DS_SUPPORT_TELEGRAM
  String t_token;
  String t_chat_id;
  auto t_token_ok = false;
  auto t_chat_id_ok = false;
  auto t_active = false;
#endif // DS_SUPPORT_TELEGRAM

  for (unsigned int i = 0; i < (unsigned int)System::web_server.args(); i++) {
    String arg_name = System::web_server.argName(i);
    if (arg_name == "action") {
      action = System::web_server.arg(i);
      action_ok = true;
    } else
    if (arg_name == "g_url") {
      g_url = System::web_server.arg(i);
      g_url_ok = true;
    } else
    if (arg_name == "g_active")
      g_active = true;
#ifdef DS_SUPPORT_TELEGRAM
    else
    if (arg_name == "t_token") {
      t_token = System::web_server.arg(i);
      t_token_ok = true;
    } else
    if (arg_name == "t_chat_id") {
      t_chat_id = System::web_server.arg(i);
      t_chat_id_ok = true;
    } else
    if (arg_name == "t_active")
      t_active = true;
#endif // DS_SUPPORT_TELEGRAM
  }

  if (action_ok) {
    if (action == "save") {
      if (g_url_ok) {
        const auto g_url_old = google_assistant.getURL();
        const auto g_active_old = google_assistant.isActive();
        if (g_url != g_url_old || g_active != g_active_old) {
          google_assistant.save(g_url, g_active);

          String lmsg = F("Google Assistant ");
          if (g_active != g_active_old) {
            lmsg += g_active ? F("") : F("de");
            lmsg += F("activated");
          }
          if (g_url != g_url_old) {
            if (g_active != g_active_old)
              lmsg += F(" and ");
            lmsg += F("URL updated from \"");
            lmsg += g_url_old;
            lmsg += F("\" to \"");
            lmsg += g_url;
            lmsg += F("\"");
          }
          lmsg += F(" from ");
          lmsg += System::web_server.client().remoteIP().toString();
          System::appLogWriteLn(lmsg, true);
        }
      }

#ifdef DS_SUPPORT_TELEGRAM
      if (t_token_ok && t_chat_id_ok) {
        const auto t_token_old = telegram.getToken();
        const auto t_chat_id_old = telegram.getChatID();
        const auto t_active_old = telegram.isActive();
        if (t_token != t_token_old || t_chat_id != t_chat_id_old || t_active != t_active_old) {
          telegram.save(t_token, t_chat_id, t_active);

          String lmsg = F("Telegram ");
          if (t_active != t_active_old) {
            lmsg += t_active ? F("") : F("de");
            lmsg += F("activated");
          }
          if (t_token != t_token_old) {
            if (t_active != t_active_old)
              lmsg += F("; ");
            lmsg += F("token updated");
          }
          if (t_chat_id != t_chat_id_old) {
            if (t_active != t_active_old || t_token != t_token_old)
              lmsg += F("; ");
            lmsg += F("chat ID updated");
          }
          lmsg += F(" from ");
          lmsg += System::web_server.client().remoteIP().toString();
          System::appLogWriteLn(lmsg, true);
        }
      }
#endif // DS_SUPPORT_TELEGRAM

      pushHeader(F("Configuration Saved"), true);
    }
    else
    if (action == "test_google") {
      if (g_url_ok && g_url.length()) {
        if (google_assistant.sendTest(g_url)) {
          System::log->printf(TIMED("Google Assistant test message sent from %s\n"), System::web_server.client().remoteIP().toString().c_str());
          pushHeader(F("Test Message Sent"), true);
          if (google_assistant.getURL() != g_url)
            System::web_page += F("<p>If OK, do not forget to save your new settings.</p>");
        } else {
          System::log->printf(TIMED("Google Assistant test message error\n"));
          pushHeader(F("Message Sending Error"), true);
        }
      } else
        pushHeader(F("Invalid Parameters"), true);
    }
#ifdef DS_SUPPORT_TELEGRAM
    else
    if (action == "test_telegram") {
      if (t_token_ok && t_token.length() && t_chat_id_ok && t_chat_id.length()) {
        if (telegram.sendTest(t_token, t_chat_id)) {
          System::log->printf(TIMED("Telegram test message sent from %s\n"), System::web_server.client().remoteIP().toString().c_str());
          pushHeader(F("Test Message Sent"), true);
          if (telegram.getToken() != t_token || telegram.getChatID() != t_chat_id)
            System::web_page += F("<p>If OK, do not forget to save your new settings.</p>");
        } else {
          System::log->printf(TIMED("Telegram test message error\n"));
          pushHeader(F("Message Sending Error"), true);
        }
      } else
        pushHeader(F("Invalid Parameters"), true);
    }
#endif // DS_SUPPORT_TELEGRAM
    else
      pushHeader(F("Unknown action"), true);
  } else
    pushHeader(F("Invalid Parameters"), true);

  pushFooter();
  System::sendWebPage();
}

// Register web pages with web server
// Note that this function cannot be called "registerWebPages" after the System class field, otherwise it will not work for an obscure reason
static void registerPages() {
  System::web_server.on("/",        serveRoot);
  System::web_server.on("/mailbox", serveMailBox);
  System::web_server.on("/save",    serveSave);
  System::web_server.on("/conf",    serveConf);
  System::web_server.on("/confSave",serveConfSave);
  System::web_server.on("/ack",     serveAcknowledge);
}

// Hook up the registration to the system class
void (*System::registerWebPages)() = registerPages;

#endif // !DS_MAILBOX_REMOTE
