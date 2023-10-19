#ifndef DCCEXTURNOUTS_H
#define DCCEXTURNOUTS_H

#include <Arduino.h>

enum TurnoutStates {
    TurnoutClosed = 0,
    TurnoutThrown = 1,
    TurnoutResponseClosed = 'C',
    TurnoutResponseThrown = 'T',
    TurnoutClose = 0,
    TurnoutThrow = 1,
    TurnoutToggle = 2,
    TurnoutExamine = 9,
};

class Turnout {
    public:
        Turnout() {}
        Turnout(int id, TurnoutStates state);
        bool setTurnoutState(TurnoutStates action);
        TurnoutStates getTurnoutState();
        bool throwTurnout();
        bool closeTurnout();
        bool toggleTurnout();
        bool setTurnoutId(int id);
        int getTurnoutId();
        bool setTurnoutName(char* name);
        char* getTurnoutName();
        void setHasReceivedDetails();
        bool getHasReceivedDetails();

    private:
        int turnoutId;
        char* turnoutName;
        TurnoutStates turnoutState;
        bool hasReceivedDetail;
};

#endif