#include <DCCEXProtocol.h>
#include <gmock/gmock.h>

class DCCEXProtocolDelegateMock : public DCCEXProtocolDelegate {
public:
  // Notify when the server version has been received
  MOCK_METHOD(void, receivedServerVersion, (int, int, int), (override));

  // Notify when a broadcast message has been received
  MOCK_METHOD(void, receivedMessage, (char *), (override));

  // Notify when the roster list is received
  MOCK_METHOD(void, receivedRosterList, (), (override));

  // Notify when the turnout list is received
  MOCK_METHOD(void, receivedTurnoutList, (), (override));

  // Notify when the route list is received
  MOCK_METHOD(void, receivedRouteList, (), (override));

  // Notify when the turntable list is received
  MOCK_METHOD(void, receivedTurntableList, (), (override));

  // Notify when an update to a Loco object is received
  MOCK_METHOD(void, receivedLocoUpdate, (Loco *), (override));

  // Notify when a Loco broadcast is received
  MOCK_METHOD(void, receivedLocoBroadcast, (int address, int speed, Direction direction, int functionMap), (override));

  // Notify when a track power state change is received
  MOCK_METHOD(void, receivedTrackPower, (TrackPower), (override));

  // Notify when a track type change is received
  MOCK_METHOD(void, receivedTrackType, (char, TrackManagerMode, int), (override));

  // Notify when a turnout state change is received
  MOCK_METHOD(void, receivedTurnoutAction, (int, bool), (override));

  // Notify when a turntable index change is received
  MOCK_METHOD(void, receivedTurntableAction, (int, int, bool), (override));

  // Notify when a loco address is read from the programming track
  MOCK_METHOD(void, receivedReadLoco, (int), (override));

  // Notify when a CV is read from the programming track
  MOCK_METHOD(void, receivedValidateCV, (int, int), (override));

  // Notify when a CV bit is validated on the programming track
  MOCK_METHOD(void, receivedValidateCVBit, (int, int, int), (override));

  // Notify when a CV is written on the programming track
  MOCK_METHOD(void, receivedWriteCV, (int, int), (override));

  // Notify when a loco address is written on the programming track
  MOCK_METHOD(void, receivedWriteLoco, (int), (override));
};