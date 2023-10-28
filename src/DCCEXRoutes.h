#ifndef DCCEXROUTES_H
#define DCCEXROUTES_H

#include <Arduino.h>

enum RouteType {
    RouteTypeRoute = 'R',
    RouteTypeAutomation = 'A',
};

class Route {
public:
  /// @brief Constructor
  /// @param id Route ID
  Route(int id);
  
  /// @brief Get route ID
  /// @return 
  int getId();

  /// @brief Set route name
  /// @param name 
  void setName(char* name);

  /// @brief Get route name
  /// @return 
  char* getName();

  /// @brief Set route type (A automation, R route)
  /// @param type RouteType - RouteTypeAutomation or RouteTypeRoute
  /// @return 
  void setType(RouteType type);

  /// @brief Get route type (A automation, R route)
  /// @return 
  RouteType getType();

  /// @brief Get count of routes
  /// @return 
  int getCount();

  /// @brief Get first Route object
  /// @return 
  static Route* getFirst();

  /// @brief Get next Route object
  /// @return 
  Route* getNext();
  
private:
  int _id;
  char* _name;
  char _type;
  static Route* _first;
  Route* _next;
  int _count=0;

};

#endif