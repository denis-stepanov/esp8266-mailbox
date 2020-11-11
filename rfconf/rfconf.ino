/* DS mailbox automation
 * * HC-12 configuration sketch
 * (c) DNS 2020
 * Arduino IDE settings:
 * * Board: Arduino/Genuino Uno
 * Review commands sent to HC-12 before programming
 */

#include <SoftwareSerial.h>

// Configuration

//// I/O
static const int PIN_HC12_TX = 8;                // HC-12 TX
static const int PIN_HC12_RX = 9;                // HC-12 RX
//// HC-12 SET must be grounded

//// RF Com
static SoftwareSerial com(PIN_HC12_TX, PIN_HC12_RX);
static const unsigned int COM_SPEED = 9600;      // HC-12 serial line speed
//// (!) 9600 only works if HC-12 is booted with SET=LOW, or preconfigured with 9600. In other cases it responds with pre-configured speed (!)

//// Logging
static HardwareSerial &syslog = Serial;          // Syslog
static const unsigned int LOG_SPEED = 9600;      // Program log serial line speed

// Send command to HC-12 and read response
static void send_command(const char *cmd, const char *desc) {
  com.println(cmd);
  syslog.print("++ ");
  syslog.println(desc);
  syslog.println(cmd);
  delay(500);            // Give enough time for reply to arrive
  while(com.available())
    syslog.write(com.read());
}

void setup() {

  syslog.begin(LOG_SPEED);
  syslog.println();
  syslog.println("+ Started.");

  com.begin(COM_SPEED);

  // Enter configuration mode
  syslog.println("++ Entering configuration mode...");
  // No specific action since SET is already grounded
  delay(250);

  // Send configuration
  send_command("AT",         "Pinging...");
  send_command("AT+V",       "Requesting module ID...");
  send_command("AT+DEFAULT", "Resetting to factory settings...");
  send_command("AT+RX",      "Factory settings:");
  send_command("AT+FU4",     "Setting transmission mode...");
  send_command("AT+C007",    "Setting communication channel...");
  send_command("AT+RX",      "Finished. Configured settings:");
}

void loop() {
}
