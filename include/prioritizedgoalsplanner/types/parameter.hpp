#ifndef INCLUDE_PRIORITIZEDGOALSPLANNER_PARAMETER_HPP
#define INCLUDE_PRIORITIZEDGOALSPLANNER_PARAMETER_HPP

#include <string>
#include "../util/api.hpp"
#include "type.hpp"

namespace cp
{
struct Entity;
struct SetOfTypes;


struct PRIORITIZEDGOALSPLANNER_API Parameter
{
  Parameter(const std::string& pName,
            const std::shared_ptr<Type>& pType);

  Parameter(const Parameter& pOther) = default;
  Parameter(Parameter&& pOther) noexcept;
  Parameter& operator=(const Parameter& pOther) = default;
  Parameter& operator=(Parameter&& pOther) noexcept;

  bool operator<(const Parameter& pOther) const;

  bool operator==(const Parameter& pOther) const;
  bool operator!=(const Parameter& pOther) const { return !operator==(pOther); }

  static Parameter fromStr(const std::string& pStr,
                           const SetOfTypes& pSetOfTypes);
  static Parameter fromType(const std::shared_ptr<Type>& pType);
  std::string toStr() const;
  Entity toEntity() const;
  bool isAParameterToFill() const;

  std::string name;
  std::shared_ptr<Type> type;
};

} // !cp


#endif // INCLUDE_PRIORITIZEDGOALSPLANNER_PARAMETER_HPP
