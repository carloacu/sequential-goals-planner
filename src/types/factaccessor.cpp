#include <contextualplanner/types/factaccessor.hpp>
#include <stdexcept>
#include <contextualplanner/types/fact.hpp>
#include <contextualplanner/types/setofentities.hpp>
#include <contextualplanner/util/util.hpp>

namespace cp
{
struct Fact;

FactAccessor::FactAccessor(const Fact& pFact)
 : _factSignature(pFact.name())
{
  _factSignature += "(";
  bool firstArg = true;
  for (const auto& currArg : pFact.arguments())
  {
    if (currArg.type)
    {
      if (firstArg)
        firstArg = false;
      else
        _factSignature += ", ";
      _factSignature += currArg.type->name;
    }
  }
  _factSignature += ")";

  if (pFact.fluent())
  {
    if (pFact.fluent()->type)
      _factSignature += "=" + pFact.fluent()->type->name;
  }
}


void FactAccessor::conditonFactToListOfFactAccessors(
    std::list<FactAccessor>& pRes,
    const Fact& pFact)
{
  pRes.emplace_back(pFact);

  for (std::size_t i = 0; i < pFact.arguments().size(); ++i)
  {
    const auto& currArg = pFact.arguments()[i];
    if (currArg.type && currArg.isAParameterToFill())
    {
      for (const auto& currSubType : currArg.type->subTypes)
      {
        auto fact = pFact;
        fact.setArgumentType(i, currSubType);
        conditonFactToListOfFactAccessors(pRes, fact);
      }
    }
  }
}



} // !cp

