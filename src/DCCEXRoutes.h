#ifndef DCCEXROUTES_H
#define DCCEXROUTES_H

#include <Arduino.h>

enum RouteType {
    RouteTypeRoute = 'R',
    RouteTypeAutomation = 'A',
};

enum RouteState {
    RouteActive = 2,
    RouteInactive = 4,
    RouteInconsistent = 8,
};

class Route {
    public:
        Route() {}
        Route(int id);
        int getRouteId();
        void setRouteName(char* name);
        char* getRouteName();
        bool setRouteType(RouteType type);
        RouteType getRouteType();

        void setHasReceivedDetails();
        bool getHasReceivedDetails();
        
    private:
        int routeId;
        char* routeName;
        char routeType;
        bool hasReceivedDetail;
};

#endif