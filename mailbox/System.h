/* DS System definition
 * * Use capability macros to enable/disable available system features
 * (c) DNS 2020
 */

#ifndef _DS_SYSTEM_H_
#define _DS_SYSTEM_H_

// System capabilities. Define them in MySystem.h before including this header
// DS_CAP_APP_ID      - enable application identification
// DS_CAP_APP_LOG     - enable application log
// DS_CAP_SYS_LED     - enable builtin LED
// DS_CAP_SYS_LOG     - enable syslog
// DS_CAP_SYS_LOG_HW  - enable syslog on hardware serial line (UART 0 or 1)
// DS_CAP_SYS_RESET   - enable software reset interface
// DS_CAP_SYS_RTCMEM  - enable RTC memory
// DS_CAP_SYS_TIME    - enable system time
// DS_CAP_SYS_UPTIME  - enable system uptime counter
// DS_CAP_SYS_FS      - enable file system
// DS_CAP_SYS_NETWORK - enable networking
// DS_CAP_WIFIMANAGER - enable Wi-Fi configuration at runtime
// DS_CAP_MDNS        - enable mDNS
// DS_CAP_WEBSERVER   - enable web server
// DS_CAP_BUTTON      - enable button

#include <Arduino.h>          // String

// Consistency checks
#if defined(DS_CAP_SYS_LOG_HW) && !defined(DS_CAP_SYS_LOG)
#define DS_CAP_SYS_LOG
#endif // DS_CAP_SYS_LOG_HW && !DS_CAP_SYS_LOG

#if defined(DS_CAP_APP_LOG) && !defined(DS_CAP_SYS_FS)
#warning "Capability DS_CAP_APP_LOG requires DS_CAP_SYS_FS; enabling"
#define DS_CAP_SYS_FS
#endif // DS_CAP_APP_LOG && !DS_CAP_SYS_FS

#if defined(DS_CAP_WIFIMANAGER) && !defined(DS_CAP_SYS_NETWORK)
#warning "Capability DS_CAP_WIFIMANAGER requires DS_CAP_SYS_NETWORK; enabling"
#define DS_CAP_SYS_NETWORK
#endif // DS_CAP_WIFIMANAGER && !DS_CAP_SYS_NETWORK

#if defined(DS_CAP_MDNS) && !defined(DS_CAP_SYS_NETWORK)
#warning "Capability DS_CAP_MDNS requires DS_CAP_SYS_NETWORK; enabling"
#define DS_CAP_SYS_NETWORK
#endif // DS_CAP_MDNS && !DS_CAP_SYS_NETWORK

#if defined(DS_CAP_WEBSERVER) && !defined(DS_CAP_SYS_NETWORK)
#warning "Capability DS_CAP_WEBSERVER requires DS_CAP_SYS_NETWORK; enabling"
#define DS_CAP_SYS_NETWORK
#endif // DS_CAP_WEBSERVER && !DS_CAP_SYS_NETWORK

#ifdef DS_CAP_SYS_LED
#include <jled.h>                   // LED, https://github.com/jandelgado/jled
#endif // DS_CAP_SYS_LED

#ifdef DS_CAP_SYS_LOG
// Log message prefixed with time (to be used with log.printf())
#define TIMED(MSG) "%010lu: " MSG, millis()
#endif // DS_CAP_SYS_LOG

#ifdef DS_CAP_SYS_RESET
#include <user_interface.h>         // ESP interface
#endif // DS_CAP_SYS_RESET

#ifdef DS_CAP_SYS_FS
#include <FS.h>                     // File system
#endif // DS_CAP_SYS_FS

#ifdef DS_CAP_WEBSERVER
#include <ESP8266WebServer.h>       // Web server
#endif // DS_CAP_WEBSERVER

#ifdef DS_CAP_BUTTON
#include <AceButton.h>              // Button, https://github.com/bxparks/AceButton
#endif // DS_CAP_BUTTON

namespace ds {

#ifdef DS_CAP_SYS_TIME
  typedef enum {
    TIME_SYNC_NONE,                                   // Time was never synchronized
    TIME_SYNC_OK,                                     // Time is synchronized
    TIME_SYNC_DEGRADED                                // Time was synchronized but not anymore
  } time_sync_t;
#endif // DS_CAP_SYS_TIME

  // Class is just a collection of system-wide routines, so all of them are made static on purpose
  class System {

    public:

      // Shared methods that are always defined
      static void begin();                            // Initialize system
      static void update();                           // Update system
      static String getCapabilities();                // Return list of configured capabilities
      static uint32_t getVersion();                   // Get system version

      // Methods and members specific to capabilities
#ifdef DS_CAP_APP_ID
      static const char *app_name;                    // Application name
      static const char *app_version;                 // Application version
      static const char *app_build;                   // Application build identifier
      static const char *app_url;                     // Application URL
#endif // DS_CAP_APP_ID

#ifdef DS_CAP_APP_LOG
    protected:
      static size_t app_log_size;                     // Application log current size

    public:
      static File app_log;                            // Application log current file
      static size_t app_log_size_max;                 // Maximum size of application log. Setting this to 0 disables log at runtime

      static bool appLogWriteLn(const String& /* line */, bool copy_to_syslog = false); // Write a line into application log, optionally copying to syslog
#endif // DS_CAP_APP_LOG

#ifdef DS_CAP_SYS_LED
      static JLed led;                                // Builtin LED
#endif // DS_CAP_SYS_LED

#ifdef DS_CAP_SYS_LOG
      static Print *log;                              // System log
#endif // DS_CAP_SYS_LOG

#ifdef DS_CAP_SYS_RESET
      static uint32 getResetReason();                 // Return reset reason
      // TODO: add reason string and the reset call itself
#endif // DS_CAP_SYS_RESET

#ifdef DS_CAP_SYS_RTCMEM
      static bool getRTCMem(uint32_t* /* result */, const uint8_t idx = 0, const uint8_t num = 1); // Read 'num' 4 bytes slots from RTC memory offset 'idx' into 'result'
      static bool setRTCMem(const uint32_t* /* source */, const uint8_t idx = 0, const uint8_t num = 1); // Store 'num' 4 bytes slots into RTC memory offset 'idx' from 'source'
#endif // DS_CAP_SYS_RTCMEM

#ifdef DS_CAP_SYS_TIME
    protected:
      static time_t time_sync_time;                   // Last time the time was synchronized
      static void timeSyncHandler();                  // Time sync event handler

    public:
      static time_t getTimeSyncTime();                // Return last time sync time
      static time_sync_t getTimeSyncStatus();         // Return time sync status
      static time_t getTime();                        // Return current time
      static String getTimeStr();                     // Return current time string
      static String getTimeStr(const time_t /* t */); // Return time string for a given time
      static void (*onTimeSync)();                    // Hook to be called when time gets synchronized
#endif // DS_CAP_SYS_TIME

#ifdef DS_CAP_SYS_UPTIME
      static String getUptimeStr();                   // Return uptime as string
#ifdef DS_CAP_SYS_TIME
      static String getBootTimeStr();                 // Return boot time string
#endif // DS_CAP_SYS_TIME
#endif // DS_CAP_SYS_UPTIME

#ifdef DS_CAP_SYS_FS
      static fs::FS& fs;                              // File system control object
#endif // DS_CAP_SYS_FS

#ifdef DS_CAP_SYS_NETWORK
    public:
      static const char *hostname;                    // Hostname; also the SSID of the temporary configuration network
#ifndef DS_CAP_WIFIMANAGER
      static const char *wifi_ssid;                   // Wi-Fi SSID
      static const char *wifi_pass;                   // Wi-Fi password
#endif // !DS_CAP_WIFIMANAGER

      static void connectNetwork(                     // Connect to a known network. LED can be used to signal connection progress
#ifdef DS_CAP_SYS_LED
        JLed *led = nullptr
#endif // DS_CAP_SYS_LED
      );
      static String getNetworkName();                 // Return network name
      static String getNetworkDetails();              // Return network details
      static String getLocalIPAddress();              // Return assigned IP address
#ifdef DS_CAP_SYS_TIME
      static const char *time_server;                 // NTP server (defaults to pool.ntp.org)
      static String getTimeServer();                  // Return NTP server name (possibly, overridden via DHCP)
#endif // DS_CAP_SYS_TIME
      static bool networkIsConnected();               // Return true if network connected
#endif // DS_CAP_SYS_NETWORK

#ifdef DS_CAP_WIFIMANAGER
    protected:
      static bool need_network_config;                // True if network configuration is required

    public:
      static void configureNetwork();                 // Configure network (blocking)
      static bool needsNetworkConfiguration();        // Return true if network needs configuration
      static void requestNetworkConfiguration();      // Request network configuration
      static String getNetworkConfigPassword();       // Return Wi-Fi configuration password
#endif // DS_CAP_WIFIMANAGER

#ifdef DS_CAP_WEBSERVER
    protected:
      static void serveAbout();                       // Serve the "about" page
#ifdef DS_CAP_APP_LOG
      static void serveAppLog();                      // Serve the "log" page
#endif // DS_CAP_APP_LOG

    public:
      static ESP8266WebServer web_server;             // Web server
      static String web_page;                         // Web page buffer
      static void pushHTMLHeader(const String& title = "", const String& head_user = "", bool redirect = false);  // Add standard header to the web page
      static void pushHTMLFooter();                   // Add standard footer to the web page
      static void (*registerWebPages)();              // Hook for registering user-supplied pages

      static void sendWebPage();                      // Send a web page
#endif // DS_CAP_WEBSERVER

#ifdef DS_CAP_BUTTON
    protected:
      static void buttonEventHandler(ace_button::AceButton* /* button */, uint8_t /* event_type */, uint8_t /* button_state */); // Button handler

    public:
      static ace_button::AceButton button;            // Builtin button on pin BUTTON_BUILTIN (0 by default)
      static void (*onButtonInit)();                  // User code to initialize button
      static void (*onButtonPress)(ace_button::AceButton* /* button */, uint8_t /* event_type */, uint8_t /* button_state */); // Hook to be called when button is operated
#endif // DS_CAP_BUTTON

  };

} // namespace ds

#endif // _DS_SYSTEM_H_
