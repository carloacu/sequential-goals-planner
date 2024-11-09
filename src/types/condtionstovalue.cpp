#include <prioritizedgoalsplanner/types/condtionstovalue.hpp>
#include <prioritizedgoalsplanner/types/condition.hpp>
#include <prioritizedgoalsplanner/types/factoptional.hpp>

namespace cp
{


bool ConditionsToValue::add(const Condition& pCondition,
                            const std::string& pValue)
{
  bool hasAddedAFact = false;
  pCondition.forAll(
        [&](const FactOptional& pFactOptional,
        bool pIgnoreFluent)
  {
    if (pFactOptional.isFactNegated)
    {
      _notFactsToValue.add(pFactOptional.fact, pValue, pIgnoreFluent);
    }
    else
    {
      _factsToValue.add(pFactOptional.fact, pValue, pIgnoreFluent);
      hasAddedAFact = true;
    }
    return ContinueOrBreak::CONTINUE;
  }
  );
  return hasAddedAFact;
}


void ConditionsToValue::erase(const std::string& pValue)
{
  _factsToValue.erase(pValue);
  _notFactsToValue.erase(pValue);
}



} // !cp

