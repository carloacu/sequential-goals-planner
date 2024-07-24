#ifndef INCLUDE_CONTEXTUALPLANNER_ENTITY_HPP
#define INCLUDE_CONTEXTUALPLANNER_ENTITY_HPP

#include <string>
#include "../util/api.hpp"
#include "type.hpp"

namespace cp
{
struct SetOfTypes;


struct CONTEXTUALPLANNER_API Entity
{
  Entity(const std::string& pValue,
         const std::shared_ptr<Type>& pType = {});

  Entity(const Entity& pOther) = default;
  Entity(Entity&& pOther) noexcept;
  Entity& operator=(const Entity& pOther) = default;
  Entity& operator=(Entity&& pOther) noexcept;

  bool operator<(const Entity& pOther) const;

  bool operator==(const Entity& pOther) const;
  bool operator!=(const Entity& pOther) const { return !operator==(pOther); }

  static Entity createAnyEntity();
  static Entity fromStr(const std::string& pStr,
                        const SetOfTypes& pSetOfTypes);
  std::string toStr() const;
  bool isAParameterToFill() const;

  std::string value;
  std::shared_ptr<Type> type;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_ENTITY_HPP
