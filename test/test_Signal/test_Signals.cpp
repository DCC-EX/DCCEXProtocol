/* -*- c++ -*-
 *
 * DCCEXProtocol
 *
 * This package implements a DCCEX native protocol connection,
 * allow a device to communicate with a DCC-EX EX-CommandStation.
 *
 * Copyright © 2024 Peter Cole
 *
 * This work is licensed under the Creative Commons Attribution-ShareAlike
 * 4.0 International License. To view a copy of this license, visit
 * http://creativecommons.org/licenses/by-sa/4.0/ or send a letter to
 * Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
 *
 * Attribution — You must give appropriate credit, provide a link to the
 * license, and indicate if changes were made. You may do so in any
 * reasonable manner, but not in any way that suggests the licensor
 * endorses you or your use.
 *
 * ShareAlike — If you remix, transform, or build upon the material, you
 * must distribute your contributions under the same license as the
 * original.
 *
 * All other rights reserved.
 *
 */

#include "../setup/SignalTests.h"

TEST_F(SignalTests, createSingleSignal) {
  // Create a signal 100
  Signal *signal100 = new Signal(100);
  signal100->setName("Signal 100");
  signal100->setState(SignalStateGreen);

  // Validate signal details
  EXPECT_EQ(signal100->getId(), 100);
  EXPECT_EQ(signal100->getState(), SignalStateGreen);

  // Validate it's in the list by ID
  EXPECT_EQ(Signal::getById(100), signal100);

  _dccexProtocol.clearSignalList();
}


 TEST_F(SignalTests, createSignalList) {
  // Create three Signals
  Signal *Signal100 = new Signal(100);
  Signal100->setName("Signal 100");
  Signal100->setState(SignalStateGreen);
  Signal *Signal101 = new Signal(101);
  Signal101->setName("Signal 101");
  Signal101->setState(SignalStateAmber);
  Signal *Signal102 = new Signal(102);
  Signal102->setName("Signal 102");
  Signal102->setState(SignalStateRed);

  // Validate Signals are in the list
  EXPECT_EQ(Signal::getById(100), Signal100);
  EXPECT_EQ(Signal::getById(101), Signal101);
  EXPECT_EQ(Signal::getById(102), Signal102);

  // Validate Signal details
  EXPECT_EQ(Signal100->getId(), 100);
  EXPECT_EQ(Signal100->getState(), SignalStateGreen);
  EXPECT_STREQ(Signal100->getName(), "Signal 100");

  // Validate Signal details
  EXPECT_EQ(Signal101->getId(), 101);
  EXPECT_EQ(Signal101->getState(), SignalStateAmber);
  EXPECT_STREQ(Signal101->getName(), "Signal 101");

  // Validate Signal details
  EXPECT_EQ(Signal102->getId(), 102);
  EXPECT_EQ(Signal102->getState(), SignalStateRed);
  EXPECT_STREQ(Signal102->getName(), "Signal 102");

   _dccexProtocol.clearSignalList();
}

/**
 * @brief Test deleting a middle signal in the list preserves the remaining list
 */
TEST_F(SignalTests, TestDeleteMiddleSignal) {
  // Create three signals
  Signal *signal100 = new Signal(100);
  signal100->setName("Signal 100");
  signal100->setState(SignalStateGreen);
  Signal *signal101 = new Signal(101);
  signal101->setName("Signal 101");
  signal101->setState(SignalStateAmber);
  Signal *signal102 = new Signal(102);
  signal102->setName("Signal 102");
  signal102->setState(SignalStateRed);

  // Validate the initial list
  ASSERT_EQ(Signal::getFirst(), signal100);
  EXPECT_EQ(signal100->getNext(), signal101);
  EXPECT_EQ(signal101->getNext(), signal102);
  EXPECT_EQ(signal102->getNext(), nullptr);

  // Delete the middle of the list
  delete signal101;

  // The remaining list must be linked directly and intact
  ASSERT_EQ(Signal::getFirst(), signal100);
  EXPECT_EQ(signal100->getNext(), signal102);
  EXPECT_EQ(signal102->getNext(), nullptr);
  EXPECT_EQ(signal100->getId(), 100);
  EXPECT_EQ(signal102->getId(), 102);

  // The deleted signal must no longer be reachable from the list head
  Signal *current = Signal::getFirst();
  while (current) {
    EXPECT_NE(current, signal101);
    current = current->getNext();
  }

  _dccexProtocol.clearSignalList();
}

/**
 * @brief Test getting a signal by ID returns the matching signal
 */
TEST_F(SignalTests, getSignalById) {
  // Create a signal to find
  Signal *signal100 = new Signal(100);
  signal100->setName("Signal 100");
  signal100->setState(SignalStateGreen);

  // Get it by ID
  EXPECT_EQ(_dccexProtocol.getSignalById(100), signal100);
  _dccexProtocol.clearSignalList();
}

/**
 * @brief Test getting a signal by an unknown ID returns nullptr
 */
TEST_F(SignalTests, getSignalByIdNotFound) {
  // No signals exist, so it must not be found
  EXPECT_EQ(_dccexProtocol.getSignalById(100), nullptr);

  Signal *signal100 = new Signal(100);
  EXPECT_EQ(_dccexProtocol.getSignalById(100), signal100);

  delete signal100;
  EXPECT_EQ(_dccexProtocol.getSignalById(100), nullptr);
  
  _dccexProtocol.clearSignalList();
}

/**
 * @brief Test getting a signal by an unknown ID walks the populated list and returns nullptr
 */
TEST_F(SignalTests, getSignalByIdNotFoundWalksList) {
  // Create a signal so the list is walked
  Signal *signal100 = new Signal(100);

  // An unknown ID walks the whole list and returns nullptr
  EXPECT_EQ(_dccexProtocol.getSignalById(200), nullptr);

  // Clean up
  delete signal100;
  EXPECT_EQ(Signal::getFirst(), nullptr);
  
  _dccexProtocol.clearSignalList();
}

/**
 * @brief Test clearing the signal list removes all signals and resets the count
 */
TEST_F(SignalTests, clearSignalListClearsAllSignals) {
  // Populate the signal list via inbound <jS id,.id> response
  _stream << "<jS 100 101>";
  _dccexProtocol.check();

  // send 2 responses to set the state of the signals
  EXPECT_CALL(_delegate, receivedSignalList()).Times(1);

  _stream << R"(<jS 100 G "Signal 100">)";
  _dccexProtocol.check();
  _stream << R"(<jS 101 A "Signal 101">)";
  _dccexProtocol.check();

  ASSERT_EQ(_dccexProtocol.getSignalCount(), 2);
  ASSERT_NE(Signal::getFirst(), nullptr);

  // Clearing the list must remove every signal and reset the count
  _dccexProtocol.clearSignalList();
  EXPECT_EQ(_dccexProtocol.getSignalCount(), 0);
  EXPECT_EQ(Signal::getFirst(), nullptr);
  EXPECT_EQ(_dccexProtocol.getSignalById(100), nullptr);
  EXPECT_EQ(_dccexProtocol.getSignalById(101), nullptr);
  
  _dccexProtocol.clearSignalList();
}

/**
 * @brief Test Signal::clearSignalList() deletes the whole list, and is safe on an empty list
 */
TEST_F(SignalTests, signalClearSignalListEmptiesList) {
  // Create three signals
  Signal *signal100 = new Signal(100);
  signal100->setName("Signal 100");
  Signal *signal101 = new Signal(101);
  signal101->setName("Signal 101");
  Signal *signal102 = new Signal(102);
  signal102->setName("Signal 102");

  // Validate the initial list
  ASSERT_EQ(Signal::getFirst(), signal100);
  EXPECT_EQ(signal100->getNext(), signal101);
  EXPECT_EQ(signal101->getNext(), signal102);
  EXPECT_EQ(signal102->getNext(), nullptr);

  // Clearing the list must delete every signal and reset the head
  Signal::clearSignalList();
  EXPECT_EQ(Signal::getFirst(), nullptr);

  // Clearing an already empty list must be safe and not crash
  EXPECT_NO_FATAL_FAILURE(Signal::clearSignalList());
  EXPECT_EQ(Signal::getFirst(), nullptr);
  
  _dccexProtocol.clearSignalList();
}

/**
 * @brief Test refreshSignalList() clears the list, resets the flags, and re-requests on getLists()
 */
TEST_F(SignalTests, refreshSignalListResetsAndReRequests) {
  // Request and receive the signal list
  _dccexProtocol.getLists(false, false, false, false, true);
  _getListsGetServerVersion();

  _dccexProtocol.getLists(false, false, false, false, true);
  EXPECT_EQ(_stream.getOutput(), "<J S>");
  _stream.clearOutput();

  EXPECT_CALL(_delegate, receivedSignalList()).Times(Exactly(2));
  _stream << "<jS 100 101>";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J S 100>");
  _stream.clearOutput();

  _stream << R"(<jS 100 G 4 "Signal 100">)";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J S 101>");
  _stream.clearOutput();

  _stream << R"(<jS 101 A "Signal 101">)";
  _dccexProtocol.check();
  _stream.clearOutput();

  ASSERT_EQ(_dccexProtocol.getSignalCount(), 2);
  EXPECT_TRUE(_dccexProtocol.receivedSignalList());

  // Refreshing must clear the list and reset the received/requested flags
  _dccexProtocol.refreshSignalList();
  EXPECT_EQ(_dccexProtocol.getSignalCount(), 0);
  EXPECT_FALSE(_dccexProtocol.receivedSignalList());
  EXPECT_FALSE(_dccexProtocol.receivedLists());

  // A fresh getLists() must request the signal list again
  _dccexProtocol.getLists(false, false, false, false, true);
  EXPECT_EQ(_stream.getOutput(), "<J S>");
  _stream.clearOutput();

  // Receiving a new list must populate a fresh signal list
  _stream << "<jS 200 201>";
  _dccexProtocol.check();

  _stream << R"(<jS 200 R 12 "Signal 200">)";
  _dccexProtocol.check();
  _stream << R"(<jS 201 G "Signal 201">)";
  _dccexProtocol.check();
  _stream.clearOutput();

  EXPECT_EQ(_dccexProtocol.getSignalCount(), 2);
  EXPECT_TRUE(_dccexProtocol.receivedSignalList());
  ASSERT_TRUE(_dccexProtocol.getSignalById(200));
  EXPECT_EQ(_dccexProtocol.getSignalById(200)->getState(), SignalStateRed);
  EXPECT_EQ(_dccexProtocol.getSignalById(201)->getState(), SignalStateGreen);
  EXPECT_EQ(_dccexProtocol.getSignalById(200)->getAspect(), 12);
  EXPECT_EQ(_dccexProtocol.getSignalById(201)->getAspect(), InvalidAspect);
 
  _dccexProtocol.clearSignalList();
}

/**
 * @brief Test setNext() directly relinks a signal in the list
 */
TEST_F(SignalTests, setNextLinksSignalDirectly) {
  // Create three signals
  Signal *signal100 = new Signal(100);
  Signal *signal101 = new Signal(101);
  Signal *signal102 = new Signal(102);

  // Default construction chains them in insertion order
  EXPECT_EQ(signal100->getNext(), signal101);
  EXPECT_EQ(signal101->getNext(), signal102);
  EXPECT_EQ(signal102->getNext(), nullptr);

  // setNext() must directly relink a signal
  signal101->setNext(signal100);
  EXPECT_EQ(signal101->getNext(), signal100);

  // Restore the list for correct teardown
  signal101->setNext(signal102);
  EXPECT_EQ(signal101->getNext(), signal102);
  
  _dccexProtocol.clearSignalList();
}

/**
 * @brief Test deleting a signal that is not in the list is a safe no-op
 */

TEST_F(SignalTests, setSignalNameAndGetName) {
  // Create a signal
  Signal *signal100 = new Signal(100);
  signal100->setState(SignalStateGreen);

  // Set the name of the signal
  const char *name = "Signal 100";
  signal100->setName(name);

  // Validate the name was set correctly
  EXPECT_STREQ(signal100->getName(), name);

  // set null name
  signal100->setName(nullptr) ;
  EXPECT_STREQ(signal100->getName(), name);

  // set another name
  signal100->setName("New Signal 100");
  EXPECT_STREQ(signal100->getName(), "New Signal 100");

  // Clean up
  _dccexProtocol.clearSignalList();
}

/**
 * @brief Test setting state and aspect for signal
 */
TEST_F(SignalTests, setSignalStateAndAspect) {
  _dccexProtocol.getLists(false, false, false, false, true);
  _getListsGetServerVersion();

  _dccexProtocol.getLists(false, false, false, false, true);
  EXPECT_EQ(_stream.getOutput(), "<J S>");
  _stream.clearOutput();

  _stream << "<jS 100 200>";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J S 100>");
  _stream.clearOutput();

  // send invalid State
  _stream << "<jS 100 X 3 \"Signal100\">";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J S 200>");
  _stream.clearOutput();

  // send invalid aspect
  _stream << "<jS 200 G \"Signal200\">";
  _dccexProtocol.check();
  
  EXPECT_EQ(_dccexProtocol.getSignalById(100)->getState(), SignalStateInvalid);
  EXPECT_EQ(_dccexProtocol.getSignalById(200)->getState(), SignalStateGreen);
  EXPECT_EQ(_dccexProtocol.getSignalById(100)->getAspect(), 3);
  EXPECT_EQ(_dccexProtocol.getSignalById(200)->getAspect(), InvalidAspect);
 
  _dccexProtocol.clearSignalList();
}
 
/**
 * @brief Test deleting a signal that is not in the list is a safe no-op
 */
TEST_F(SignalTests, removeSignalNotInListLeavesListIntact) {
  // Create three signals
  Signal *signal100 = new Signal(100);
  Signal *signal101 = new Signal(101);
  Signal *signal102 = new Signal(102);

  // Validate the initial list
  ASSERT_EQ(Signal::getFirst(), signal100);
  EXPECT_EQ(signal100->getNext(), signal101);
  EXPECT_EQ(signal101->getNext(), signal102);

  // Orphan the middle and trailing signals by severing the link from the head
  signal100->setNext(nullptr);

  // Deleting a signal that is not in the list must not crash or alter the list
  EXPECT_NO_FATAL_FAILURE(delete signal101);
  EXPECT_EQ(Signal::getFirst(), signal100);
  EXPECT_EQ(signal100->getNext(), nullptr);

  // The trailing orphan is also not in the list and must be safe to delete
  EXPECT_NO_FATAL_FAILURE(delete signal102);
  EXPECT_EQ(Signal::getFirst(), signal100);
  EXPECT_EQ(signal100->getNext(), nullptr);

  // Deleting the remaining head signal empties the list
  delete signal100;
  EXPECT_EQ(Signal::getFirst(), nullptr);
}

/**
 * @brief Test receiving signal state broadcast
 */

TEST_F(SignalTests, receiveSignalListAndState) {
  // Request and receive the signal list
  _dccexProtocol.getLists(false, false, false, false, true);
  _getListsGetServerVersion();

  _dccexProtocol.getLists(false, false, false, false, true);
  EXPECT_EQ(_stream.getOutput(), "<J S>");
  _stream.clearOutput();

  // send signal list response
  _stream << "<jS 100 101>";
  _dccexProtocol.check();

  ASSERT_EQ(_dccexProtocol.getSignalCount(), 2);
  EXPECT_FALSE(_dccexProtocol.receivedSignalList());
  EXPECT_CALL(_delegate, receivedSignalList()).Times(Exactly(1));

  // send first signal response
  _stream << "<jS 100 G 9 \"Signal 100\">";
  _dccexProtocol.check();

 // send second signal response
  _stream << R"(<jS 101 R 12 "Signal 101">)";
  _dccexProtocol.check();

  EXPECT_EQ(_dccexProtocol.signals->getById(100)->getState(), SignalStateGreen);
  EXPECT_EQ(_dccexProtocol.signals->getById(101)->getState(), SignalStateRed);
  EXPECT_EQ(_dccexProtocol.signals->getById(100)->getAspect(), 9);
  EXPECT_EQ(_dccexProtocol.signals->getById(101)->getAspect(), 12);
  
  _dccexProtocol.clearSignalList();
}


TEST_F(SignalTests, receiveSignalBroadcast) {
   // Request and receive the signal list
  _dccexProtocol.getLists(false, false, false, false, true);
  _getListsGetServerVersion();
  
  _dccexProtocol.getLists(false, false, false, false, true);
  EXPECT_EQ(_stream.getOutput(), "<J S>");
  _stream.clearOutput();

  EXPECT_CALL(_delegate, receivedSignalList()).Times(Exactly(1));
  EXPECT_CALL(_delegate, receivedSignalState(200, SignalStateGreen, 5)).Times(Exactly(1));
  EXPECT_CALL(_delegate, receivedSignalState(201, SignalStateAmber, InvalidAspect)).Times(Exactly(1));

  // send first signal broadcast
  _stream << "<jS 200 201>";
  _dccexProtocol.check();

 // send first signal response
  _stream << "<jS 200 G 5 \"Signal 200\">";
  _dccexProtocol.check();

 // send second signal response
  _stream << R"(<jS 201 A "Signal 201">)";
  _dccexProtocol.check();

  EXPECT_CALL(_delegate, receivedSignalState(200, SignalStateRed, 7)).Times(Exactly(1));
  EXPECT_CALL(_delegate, receivedSignalState(201, SignalStateAmber, 13)).Times(Exactly(1));

  // send broadcast
  _stream << "<h 200 R 7>";
  _dccexProtocol.check();
  _stream << "<h 201 A 13>";
  _dccexProtocol.check();

  EXPECT_EQ(_dccexProtocol.signals->getById(200)->getState(), SignalStateRed);
  EXPECT_EQ(_dccexProtocol.signals->getById(201)->getState(), SignalStateAmber);
  EXPECT_EQ(_dccexProtocol.signals->getById(200)->getAspect(), 7);
  EXPECT_EQ(_dccexProtocol.signals->getById(201)->getAspect(), 13);

  _dccexProtocol.clearSignalList();
}

/*
 * Test case where the signal list featuer is not supported 
 * by an older CS. Even if a list is requested by getList, 
 * no signal list will be received and list will remain empty.
*/
TEST_F(SignalTests, signalListNotSupportedByCS) {
  // Request and receive the signal list
  _dccexProtocol.getLists(false, false, false, false, true);
  _getListsGetServerVersion("4.10.9");    // send an older CS version.

  EXPECT_CALL(_delegate, receivedSignalList()).Times(Exactly(0));
  _dccexProtocol.getLists(false, false, false, false, true);
  EXPECT_EQ(_stream.getOutput(), "");

  EXPECT_FALSE(_dccexProtocol.receivedSignalList());
  ASSERT_EQ(_dccexProtocol.getSignalCount(), 0);
  ASSERT_EQ(Signal::getFirst(), nullptr);

  _dccexProtocol.clearSignalList();
}

