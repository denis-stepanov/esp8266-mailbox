/* DS mailbox automation
 * * Transceiver implementation
 * (c) DNS 2020
 */

#include "Transceiver.h"

using namespace ds;

// Check receiver ID
bool Transceiver::receiverIDOK() const {
  const auto rx_id = msg.getReceiverID();
  return rx_id == RECEIVER_ID || rx_id == RECEIVER_ID_ANY;                 
}
