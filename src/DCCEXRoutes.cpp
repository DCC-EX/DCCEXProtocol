#include <Arduino.h>
#include "DCCEXRoutes.h"

Route::Route(int id) {
    routeId = id;
    hasReceivedDetail = false;
    routeName = nullptr;
}

int Route::getRouteId() {
    return routeId;
}

void Route::setRouteName(char* name) {
    routeName = name;
}

char* Route::getRouteName() {
    return routeName;
}

bool Route::setRouteType(RouteType type) {
    routeType = type;
    return true;
}

RouteType Route::getRouteType() {
    return (RouteType)routeType;
}

void Route::setHasReceivedDetails() {
    hasReceivedDetail = true;
}

bool Route::getHasReceivedDetails() {
    return hasReceivedDetail;
}