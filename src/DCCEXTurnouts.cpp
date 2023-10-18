#include <Arduino.h>
#include "DCCEXTurnouts.h"

Turnout::Turnout(int id, TurnoutStates state) {
    turnoutId = id;
    turnoutState = state;
    hasReceivedDetail = false;
    turnoutName = nullptr;
}

bool Turnout::throwTurnout() {
    setTurnoutState(TurnoutThrow);
    return true;
}

bool Turnout::closeTurnout() {
    setTurnoutState(TurnoutClose);
    return true;
}

bool Turnout::toggleTurnout() {
    setTurnoutState(TurnoutToggle);
    return true;
}

bool Turnout::setTurnoutState(TurnoutStates action) {
    TurnoutStates newState = action;
    if (action == TurnoutToggle) {
        if (turnoutState == TurnoutClosed ) {
            newState = TurnoutThrown;
        } else { // Thrown or Inconsistant
            newState = TurnoutClosed;
        }
    }
    if (newState<=TurnoutThrow) { // ignore TurnoutExamine
        turnoutState = newState;
        // sendTurnoutAction(turnoutId, newState)
        return true;
    }
    return false;
}

bool Turnout::setTurnoutId(int id) {
    turnoutId = id;
    return true;
}

int Turnout::getTurnoutId() {
    return turnoutId;
}

bool Turnout::setTurnoutName(char* name) {
    turnoutName = name;
    return true;
}

char* Turnout::getTurnoutName() {
    return turnoutName;
}

TurnoutStates Turnout::getTurnoutState() {
    return turnoutState;
}

void Turnout::setHasReceivedDetails() {
    hasReceivedDetail = true;
}

bool Turnout::getHasReceivedDetails() {
    return hasReceivedDetail;
}
