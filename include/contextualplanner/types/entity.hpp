#ifndef INCLUDE_CONTEXTUALPLANNER_ENTITY_HPP
#define INCLUDE_CONTEXTUALPLANNER_ENTITY_HPP

#include <string>
#include "../util/api.hpp"


namespace cp
{

struct CONTEXTUALPLANNER_API Entity
{
  Entity(std::string pValue,
         std::string pType = "");

  static Entity createAnyEntity();

  std::string value;
  std::string type;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_ENTITY_HPP
