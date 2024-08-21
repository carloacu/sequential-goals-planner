#include <contextualplanner/types/facttoconditions.hpp>
#include <stdexcept>
#include <contextualplanner/types/factaccessor.hpp>
#include <contextualplanner/types/fact.hpp>
#include <contextualplanner/util/alias.hpp>
#include <contextualplanner/util/util.hpp>

namespace cp
{
namespace
{
const std::string _emptyString = "";

std::string _getExactCall(const Fact& pFact)
{
  auto res = pFact.name();
  res += "(";
  bool firstArg = true;
  for (const auto& currArg : pFact.arguments())
  {
    if (currArg.type)
    {
      if (firstArg)
        firstArg = false;
      else
        res += ", ";
      res += currArg.value;
    }
  }
  res += ")";
  return res;
}


void _addFluentToExactCall(std::string& pRes,
                           const Fact& pFact)
{
  if (pFact.fluent())
  {
    pRes += "=";
    if (pFact.isValueNegated())
      pRes = "!";
    pRes += pFact.fluent()->value;
  }
}
}



FactToConditions::FactToConditions()
 : _values(),
   _valueToFacts(),
   _exactCallToListsOpt(),
   _exactCallWithoutFluentToListsOpt(),
   _signatureToLists(),
   _valuesWithoutFact()
{
}


void FactToConditions::add(const Fact& pFact,
                           const std::string& pValue,
                           bool pIgnoreFluent)
{
  auto insertionResult = _values.insert(pValue);
  _valueToFacts[pValue].emplace_back(pFact);

  if (!pFact.hasAParameter())
  {
    auto exactCallStr = _getExactCall(pFact);
    if (!_exactCallWithoutFluentToListsOpt)
      _exactCallWithoutFluentToListsOpt.emplace();
    (*_exactCallWithoutFluentToListsOpt)[exactCallStr].emplace_back(pValue);

    if (!pIgnoreFluent && pFact.fluent())
    {
      _addFluentToExactCall(exactCallStr, pFact);
      if (!_exactCallToListsOpt)
        _exactCallToListsOpt.emplace();
      (*_exactCallToListsOpt)[exactCallStr].emplace_back(pValue);
    }
  }

  std::list<FactAccessor> accessors;
  FactAccessor::conditonFactToListOfFactAccessors(accessors, pFact);
  for (auto& currAccessor : accessors)
  {
    auto& factArguments = pFact.arguments();
    auto insertionRes = _signatureToLists.emplace(currAccessor.factSignature(), factArguments.size());
    ParameterToValues& parameterToValues = insertionRes.first->second;

    parameterToValues.all.emplace_back(pValue);
    for (std::size_t i = 0; i < factArguments.size(); ++i)
    {
      if (!factArguments[i].isAParameterToFill())
        parameterToValues.argIdToArgValueToValues[i][factArguments[i].value].emplace_back(pValue);
      else
        parameterToValues.argIdToArgValueToValues[i][""].emplace_back(pValue);
    }
    if (pIgnoreFluent || pFact.fluent())
    {
      if (!pIgnoreFluent && !pFact.fluent()->isAParameterToFill() && !pFact.isValueNegated())
        parameterToValues.fluentValueToValues[pFact.fluent()->value].emplace_back(pValue);
      else
        parameterToValues.fluentValueToValues[""].emplace_back(pValue);
    }
  }
}


void FactToConditions::addValueWithoutFact(const std::string& pValue)
{
  auto insertionResult = _values.insert(pValue);
  // pValue well added and did not already exists
  if (insertionResult.second)
  {
    _valuesWithoutFact.emplace_back(pValue);
  }
}


void FactToConditions::_erase(const Fact& pFact,
                              const std::string& pValue)
{
  auto it = _values.find(pValue);
  if (it != _values.end())
  {
    if (!pFact.hasAParameter())
    {
      auto exactCallStr = _getExactCall(pFact);
      if (_exactCallWithoutFluentToListsOpt)
      {
        std::list<std::string>& listOfTypes = (*_exactCallWithoutFluentToListsOpt)[exactCallStr];
        _removeAValueForList(listOfTypes, pValue);
        if (listOfTypes.empty())
          _exactCallWithoutFluentToListsOpt->erase(exactCallStr);
      }

      if (_exactCallToListsOpt && pFact.fluent())
      {
        _addFluentToExactCall(exactCallStr, pFact);
        std::list<std::string>& listWithFluentOfTypes = (*_exactCallToListsOpt)[exactCallStr];
        _removeAValueForList(listWithFluentOfTypes, pValue);
        if (listWithFluentOfTypes.empty())
          _exactCallToListsOpt->erase(exactCallStr);
      }
    }

    std::list<FactAccessor> accessors;
    FactAccessor::conditonFactToListOfFactAccessors(accessors, pFact);
    for (auto& currAccessor : accessors)
    {
      auto& factArguments = pFact.arguments();
      const auto& currSignature = currAccessor.factSignature();
      auto itParameterToValues = _signatureToLists.find(currSignature);
      if (itParameterToValues != _signatureToLists.end())
      {
        ParameterToValues& parameterToValues = itParameterToValues->second;

        _removeAValueForList(parameterToValues.all, pValue);
        if (parameterToValues.all.empty())
        {
          _signatureToLists.erase(currSignature);
        }
        else
        {
          for (std::size_t i = 0; i < factArguments.size(); ++i)
          {
            const std::string& argKey = !factArguments[i].isAParameterToFill() ? factArguments[i].value : _emptyString;
            std::list<std::string>& listOfValues = parameterToValues.argIdToArgValueToValues[i][argKey];
            _removeAValueForList(listOfValues, pValue);
            if (listOfValues.empty())
               parameterToValues.argIdToArgValueToValues[i].erase(argKey);
          }
          if (pFact.fluent())
          {
            const std::string& fluentKey = !pFact.fluent()->isAParameterToFill() ? pFact.fluent()->value : _emptyString;
            std::list<std::string>& listOfValues = parameterToValues.fluentValueToValues[fluentKey];
            _removeAValueForList(listOfValues, pValue);
            if (listOfValues.empty())
               parameterToValues.fluentValueToValues.erase(fluentKey);
          }
        }
      }
      else
      {
        throw std::runtime_error("Errur while deleteing a fact link");
      }
    }
  }
}


void FactToConditions::erase(const std::string& pValue)
{
  auto it = _valueToFacts.find(pValue);
  if (it != _valueToFacts.end())
  {
    for (auto& currFact : it->second)
    {
      _erase(currFact, pValue);
    }
    _valueToFacts.erase(it);

    auto itVal = _values.find(pValue);
    if (itVal != _values.end())
    {
      _removeAValueForList(_valuesWithoutFact, pValue);
      _values.erase(itVal);
    }
  }
}


void FactToConditions::clear()
{
  _values.clear();
  _valueToFacts.clear();
  if (_exactCallToListsOpt)
    _exactCallToListsOpt.reset();
  if (_exactCallWithoutFluentToListsOpt)
    _exactCallWithoutFluentToListsOpt.reset();
  _signatureToLists.clear();
}


bool FactToConditions::empty() const
{
  return _values.empty();
}


typename FactToConditions::ConstMapOfFactIterator FactToConditions::find(const Fact& pFact,
                                                                         bool pIgnoreFluent) const
{
  const std::list<std::string>* exactMatchPtr = nullptr;

  if (!pFact.hasAParameter(pIgnoreFluent) && !pFact.isValueNegated())
  {
    auto exactCallStr = _getExactCall(pFact);
    if (!pIgnoreFluent && pFact.fluent())
    {
      _addFluentToExactCall(exactCallStr, pFact);
      exactMatchPtr = _findAnExactCall(_exactCallToListsOpt, exactCallStr);
    }
    else
    {
      exactMatchPtr = _findAnExactCall(_exactCallWithoutFluentToListsOpt, exactCallStr);
    }
  }

  const std::list<std::string>* resPtr = nullptr;
  auto _matchArg = [&](const std::map<std::string, std::list<std::string>>& pArgValueToValues,
                       const std::string& pArgValue) -> std::optional<typename FactToConditions::ConstMapOfFactIterator> {
    auto itForThisValue = pArgValueToValues.find(pArgValue);
    if (itForThisValue != pArgValueToValues.end())
    {
      auto itForAnyValue = pArgValueToValues.find("");
      if (itForAnyValue != pArgValueToValues.end())
      {
        if (exactMatchPtr != nullptr)
          return ConstMapOfFactIterator(mergeTwoListsWithNoDoubleEltCheck(*exactMatchPtr, itForAnyValue->second));
        return ConstMapOfFactIterator(mergeTwoLists(itForThisValue->second, itForAnyValue->second));
      }
      if (exactMatchPtr == nullptr)
      {
        if (resPtr != nullptr)
          return ConstMapOfFactIterator(intersectTwoLists(*resPtr, itForThisValue->second));
        resPtr = &itForThisValue->second;
      }
    }
    else
    {
      auto itForAnyValue = pArgValueToValues.find("");
      if (itForAnyValue != pArgValueToValues.end())
      {
        if (resPtr != nullptr)
          return ConstMapOfFactIterator(intersectTwoLists(*resPtr, itForAnyValue->second));
        resPtr = &itForAnyValue->second;
      }
      else
      {
        return ConstMapOfFactIterator(nullptr);
      }
    }
    return {};
  };


  auto factAccessor = pFact.toFactAccessor();
  auto itParameterToValues = _signatureToLists.find(factAccessor.factSignature());
  if (itParameterToValues != _signatureToLists.end())
  {
    const ParameterToValues& parameterToValues = itParameterToValues->second;

    bool hasOnlyParameters = true;
    auto& factArguments = pFact.arguments();
    for (std::size_t i = 0; i < factArguments.size(); ++i)
    {
      if (!factArguments[i].isAParameterToFill())
      {
        hasOnlyParameters = false;
        auto subRes = _matchArg(parameterToValues.argIdToArgValueToValues[i], factArguments[i].value);
        if (subRes)
          return *subRes;
      }
    }

    const auto& factFluent = pFact.fluent();
    if (!pIgnoreFluent && factFluent)
    {
      if (!factFluent->isAParameterToFill() && !pFact.isValueNegated())
      {
        hasOnlyParameters = false;
        auto subRes = _matchArg(parameterToValues.fluentValueToValues, factFluent->value);
        if (subRes)
          return *subRes;
      }
    }

    if (hasOnlyParameters)
      return ConstMapOfFactIterator(&parameterToValues.all);
  }

  if (resPtr != nullptr)
    return ConstMapOfFactIterator(resPtr);
  return ConstMapOfFactIterator(exactMatchPtr);
}

typename FactToConditions::ConstMapOfFactIterator FactToConditions::valuesWithoutFact() const
{
  return ConstMapOfFactIterator(&_valuesWithoutFact);
}



void FactToConditions::_removeAValueForList(std::list<std::string>& pList,
                                            const std::string& pValue) const
{
  for (auto it = pList.begin(); it != pList.end(); ++it)
  {
    if (*it == pValue)
    {
      pList.erase(it);
      return;
    }
  }
}


const std::list<std::string>* FactToConditions::_findAnExactCall(
    const std::optional<std::map<std::string, std::list<std::string>>>& pExactCalls,
    const std::string& pExactCall) const
{
  if (pExactCalls)
  {
    auto it = pExactCalls->find(pExactCall);
    if (it != pExactCalls->end())
      return &it->second;
  }
  return nullptr;
}


} // !cp

