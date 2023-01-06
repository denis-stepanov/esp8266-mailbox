/* DS mailbox automation
 * * Local module
 * * * Mailbox definition
 * (c) DNS 2020-2023
 */

#ifndef _DS_VIRTUALMAILBOX_H_
#define _DS_VIRTUALMAILBOX_H_

#include "MailBox.h"         // Base class
#include "MySystem.h"        // Timers

namespace ds {

  // Mailbox alarms, in order from lower to higher severity
  // Note that lower level alarms may override higher level under certain circumstances
  typedef enum {
    ALARM_NONE,                              // No standing alarm
    ALARM_BOOTED,                            // Module rebooted
    ALARM_BATTERY,                           // Low battery
    ALARM_ABSENT,                            // Mailbox haven't reported back for a long time
    ALARM_DOOR_FLIPPED,                      // Door was opened and then closed
    ALARM_DOOR_LEFTOPEN,                     // Door was opened and left open
    ALARM_DOOR_OPEN                          // Door is open right now
  } mailbox_alarm;
  
  // Virtual mailbox class
  class VirtualMailBox : public MailBox {

    protected:
      time_t last_seen;                      // Last time the mailbox reported
      uint32_t msg_recv;                     // Number of received messages
      mailbox_alarm alarm;                   // Alarm status
      TimerCountdownTick timer;              // Timer to check for absent second message
      bool g_opening_reported;               // True if opening has already been reported to Google
      bool low_battery_reported;             // True if low battery status has been recently reported

      static String getConfFileName(const uint8_t /* id */); // Return configuration file name (static version)
      String getConfFileName() const;        // Return configuration file name
      void timeout();                        // Message timeout handler

    public:
      static const uint8_t RADIO_RELIABILITY_BAD = 89;     // (%)
      static const unsigned int ABSENCE_TIME = 3 * 24 * 60 * 60; // Interval after which mailbox is considered absent (s). Expected to be at least 1 day

      VirtualMailBox(const uint8_t _id = 1, const String _label = (char *)nullptr, const uint8_t _battery = BATTERY_LEVEL_UNKNOWN, const time_t _last_seen = 0); // Constructor
      time_t getLastSeen() const;            // Return the last report time
      void setLastSeen(const time_t t = 0);  // Set the last report time. 0 means current time
      int8_t getRadioReliability() const;    // Return radio link reliability (%). -1 == unknown
      mailbox_alarm getAlarm() const;        // Return mailbox alarm
      String getAlarmStr(const bool html = false) const; // Return mailbox alarm as string (possibly, HTMLized)
      static String getAlarmStr(const mailbox_alarm /* a */, const bool html = false); // Return mailbox alarm as string (static version)
      String getAlarmIcon() const;           // Return alarm icon
      static String getAlarmIcon(const mailbox_alarm /* a */); // Return alarm icon (static version)
      void updateAlarm();                    // Update mailbox alarm
      void resetAlarm();                     // Reset mailbox alarm
      bool isOK();                           // Return false in degraded conditions (battery low or mailbox absent)
      void printHTML(String& /* buf */) const; // Print mailbox status in HTML
      void printText(String& /* buf */) const; // Print mailbox status in text
      void save() const;                     // Save mailbox information to disk
      static VirtualMailBox *load(const uint8_t /* id */); // Initialize mailbox with information on disk
      static bool forget(const uint8_t /* id */); // Remove mailbox information from disk
      VirtualMailBox& operator=(const MailBoxMessage& /* msg */); // Update mailbox from message data
  };

} // namespace ds

String& operator<<(String& /* html_buf */, ds::VirtualMailBox& /* mb */);  // HTML printout helper

#endif // _DS_VIRTUALMAILBOX_H_
