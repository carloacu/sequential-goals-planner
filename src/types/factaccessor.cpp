#include <contextualplanner/types/factaccessor.hpp>
#include <stdexcept>
#include <contextualplanner/types/fact.hpp>
#include <contextualplanner/types/setofentities.hpp>
#include <contextualplanner/util/util.hpp>

namespace cp
{
struct Fact;

FactAccessor::FactAccessor(const Fact& pFact,
                           bool pInAcontainer,
                           bool pIgnoreFluent)
 : _factSignature(pFact.name()),
   _hasParameters(false),
   _argumentConstantValues(),
   _fluentConstantValue(),
   _inAContainer(pInAcontainer),
   _ignoreFluent(pIgnoreFluent)
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

    if (!currArg.isAParameterToFill())
    {
      _argumentConstantValues.emplace_back(currArg.value);
    }
    else
    {
      _hasParameters = true;
      _argumentConstantValues.emplace_back("");
    }
  }
  _factSignature += ")";

  if (!_ignoreFluent && pFact.fluent())
  {
    if (pFact.fluent()->type)
      _factSignature += "=" + pFact.fluent()->type->name;

    if (!pFact.fluent()->isAParameterToFill())
    {
      if (pFact.isValueNegated())
        _fluentConstantValue = "!";
      _fluentConstantValue += pFact.fluent()->value;
    }
    else
    {
      _hasParameters = true;
    }
  }
}


void FactAccessor::conditonFactToListOfFactAccessors(
    std::list<FactAccessor>& pRes,
    const Fact& pFact)
{
  pRes.emplace_back(pFact, true);

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


bool FactAccessor::operator<(const FactAccessor& pOther) const
{
  if (_factSignature != pOther._factSignature)
    return _factSignature < pOther._factSignature;

  if (_inAContainer)
  {
    if (pOther._inAContainer)
      return _compareBothFromConditionAccessors(pOther);
    return _compareFromCondition(pOther);
  }

  if (pOther._inAContainer)
    return pOther._compareFromCondition(*this);

  if (CONTEXTUALPLANNER_DEBUG_FOR_TESTS)
    throw std::runtime_error("FactAccessor comparison without any accessor that has _mustCompareAllArgAndFluent true");
  return false;
}



bool FactAccessor::_compareBothFromConditionAccessors(const FactAccessor& pOther) const
{
  if (_argumentConstantValues.size() != pOther._argumentConstantValues.size())
    return false;

  for (std::size_t i = 0; i < _argumentConstantValues.size(); ++i)
    if (_argumentConstantValues[i] != pOther._argumentConstantValues[i])
      return _argumentConstantValues[i] < pOther._argumentConstantValues[i];

  if (!_ignoreFluent && !pOther._ignoreFluent &&
      _fluentConstantValue != pOther._fluentConstantValue)
    return _fluentConstantValue < pOther._fluentConstantValue;
  return false;
}

bool FactAccessor::_compareFromCondition(const FactAccessor& pOtherThatIsNotfromACondition) const
{
  if (_argumentConstantValues.size() != pOtherThatIsNotfromACondition._argumentConstantValues.size())
    return false;

  for (std::size_t i = 0; i < _argumentConstantValues.size(); ++i)
    if (_argumentConstantValues[i] != "" &&
        _argumentConstantValues[i] != pOtherThatIsNotfromACondition._argumentConstantValues[i])
      return _argumentConstantValues[i] < pOtherThatIsNotfromACondition._argumentConstantValues[i];

  if (!_ignoreFluent && !pOtherThatIsNotfromACondition._ignoreFluent &&
      _fluentConstantValue != "" &&
      _fluentConstantValue != pOtherThatIsNotfromACondition._fluentConstantValue)
    return _fluentConstantValue < pOtherThatIsNotfromACondition._fluentConstantValue;
  return false;
}


} // !cp

