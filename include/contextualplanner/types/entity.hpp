#ifndef INCLUDE_CONTEXTUALPLANNER_ENTITY_HPP
#define INCLUDE_CONTEXTUALPLANNER_ENTITY_HPP

#include <string>
#include <vector>
#include "../util/api.hpp"
#include "type.hpp"

namespace cp
{
struct SetOfTypes;
struct Parameter;

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

  static const std::string& anyEntityValue();
  static Entity createAnyEntity();
  static Entity createNumberEntity(const std::string& pNumber,
                                   const SetOfTypes& pSetOfTypes);
  static Entity fromStr(const std::string& pStr,
                        const SetOfTypes& pSetOfTypes);
  std::string toStr() const;
  bool isAnyValue() const;
  bool isAParameterToFill() const;
  bool match(const Parameter& pParameter) const;
  bool isValidParameterAccordingToPossiblities(const std::vector<Parameter>& pParameter) const;

  std::string value;
  std::shared_ptr<Type> type;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_ENTITY_HPP
