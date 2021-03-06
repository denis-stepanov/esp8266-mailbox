/* DS mailbox automation
 * * Local module
 * * * Web pages implementation
 * (c) DNS 2020
 */

#include "MySystem.h"               // System-level definitions

#ifndef DS_MAILBOX_REMOTE

#include "MailBoxManager.h"         // Mailbox manager

using namespace ds;

// Server data providers
extern MailBoxManager mailbox_manager;     // Mailbox manager instance

// Initialize page buffer with page header
static void pushHeader(const String& title, bool redirect = false) {
  String &page = System::web_page;
  String uri = System::web_server.uri();

  System::pushHTMLHeader(title, uri == "/" ?
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
    "</style>\n") : F(""),
    redirect
  );
  page += F("<h3>");
  page += title;
  page += F("</h3>\n");

  // Navigation
  page += F(
    "<table cellpadding=\"0\" cellspacing=\"0\" width=\"100%\"><tr><td>"
    "[ <a href=\"/\">home</a> ]&nbsp;&nbsp;&nbsp;"
    "[ <a href=\"/log\">log</a> ]&nbsp;&nbsp;&nbsp;"
    "[ <a href=\"/about\">about</a> ]&nbsp;&nbsp;&nbsp;"
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
        pushHeader((String)"Mailbox " + id);
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
    bool id_ok = false, label_ok = false, action_ok = false;
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
  String via = F("web from ");
  via += System::web_server.client().remoteIP().toString();
  mailbox_manager.acknowledgeAlarm(via);
  pushHeader(F("Alarm Acknowledged"), true);
  pushFooter();
  System::sendWebPage();
}

// Register web pages with web server
// Note that this function cannot be called "registerWebPages" after the System class field, otherwise it will not work for an obscure reason
static void registerPages() {
  System::web_server.on("/",        serveRoot);
  System::web_server.on("/mailbox", serveMailBox);
  System::web_server.on("/save",    serveSave);
  System::web_server.on("/ack",     serveAcknowledge);
}

// Hook up the registration to the system class
void (*System::registerWebPages)() = registerPages;

#endif // !DS_MAILBOX_REMOTE
