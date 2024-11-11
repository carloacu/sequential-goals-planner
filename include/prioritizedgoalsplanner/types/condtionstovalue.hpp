#ifndef INCLUDE_PRIORITIZEDGOALSPLANNER_CONDITIONSTOVALUE_HPP
#define INCLUDE_PRIORITIZEDGOALSPLANNER_CONDITIONSTOVALUE_HPP

#include "../util/api.hpp"
#include <prioritizedgoalsplanner/types/factstovalue.hpp>

namespace pgp
{
struct Condition;
struct Fact;


struct PRIORITIZEDGOALSPLANNER_API ConditionsToValue
{
public:
  bool add(const Condition& pCondition,
           const std::string& pValue);

  void erase(const std::string& pValue);

  const FactsToValue& factsToValue() const { return _factsToValue; }
  const FactsToValue& notFactsToValue() const { return _notFactsToValue; }


private:
  /// Map of facts from condition to value.
  FactsToValue _factsToValue;
  /// Map of negationed facts from condition to value.
  FactsToValue _notFactsToValue;
};

} // !pgp


#endif // INCLUDE_PRIORITIZEDGOALSPLANNER_CONDITIONSTOVALUE_HPP
