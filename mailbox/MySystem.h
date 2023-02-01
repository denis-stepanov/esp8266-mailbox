/* DS mailbox automation
 * * Local or remote module selector
 * * * System capabilities
 * (c) DNS 2020-2023
 */

#ifndef _DS_SYSTEM_H_

// Global selectors. Modify them before compiling

//// Select mailbox module (local - default, or remote). If remote, uncomment the line below
//#define DS_MAILBOX_REMOTE

//// For local module, uncomment the line below if using a development board (Witty Cloud)
//#define DS_DEVBOARD

//// Uncomment if you need Telegram interface
//#define DS_SUPPORT_TELEGRAM

#ifdef DS_MAILBOX_REMOTE

// Remote module
#define DS_CAP_SYS_RESET    // Enable software reset interface
#define DS_CAP_SYS_RTCMEM   // Enable RTC memory

#define DS_UNSTABLE_SERIAL  // Skip serial log initialization delay

#else

// Local module
#define DS_CAP_APP_LOG      // Enable application log
#define DS_CAP_SYS_LED      // Enable builtin LED
#define DS_CAP_SYS_TIME     // Enable system time
#define DS_CAP_SYS_UPTIME   // Enable system uptime counter
#define DS_CAP_SYS_FS       // Enable file system
#define DS_CAP_SYS_NETWORK  // Enable networking
#define DS_CAP_WIFIMANAGER  // Enable Wi-Fi configuration at runtime
#define DS_CAP_MDNS         // Enable mDNS
#define DS_CAP_WEBSERVER    // Enable web server
#define DS_CAP_BUTTON       // Enable button
#define DS_CAP_TIMERS_COUNT_TICK // Enable countdown timers via ticker

#define DS_LED_VS_SERIAL_CHECKED_OK   // LED on pin 1, log line on pin 2

//// Set your own timezone. See the list in TZ.h coming with ESP8266 Arduino Core
#define DS_TIMEZONE TZ_Etc_UTC

#ifdef DS_DEVBOARD
#if LED_BUILTIN != 2
#error "DEV BOARD: set Builtin LED to 2!"
#endif // LED_BUILTIN != 2
#else
#if LED_BUILTIN != 1
#error "PROD BOARD: set Builtin LED to 1!"
#endif // LED_BUILTIN != 1
#endif // DS_DEVBOARD

#ifdef DS_DEVBOARD
#define BUTTON_BUILTIN 4    // GPIO4 on Witty Cloud
// Button in prod is GPIO0, which is a default; so no need to define
#endif // DS_DEVBOARD
;

#endif // DS_MAILBOX_REMOTE

// Common for both modules
#define DS_CAP_APP_ID       // Enable application identification
#define DS_CAP_SYS_LOG_HW   // Enable syslog on hardware serial line

#include "System.h"         // System global definitions

#endif // _DS_SYSTEM_H_
