#include <contextualplanner/types/setoffacts.hpp>
#include <stdexcept>
#include <contextualplanner/types/fact.hpp>
#include <contextualplanner/util/alias.hpp>
#include <contextualplanner/util/util.hpp>

namespace cp
{
namespace
{
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



SetOfFact::SetOfFact()
 : _facts(),
   _exactCallToListsOpt(),
   _exactCallWithoutFluentToListsOpt(),
   _signatureToLists()
{
}


void SetOfFact::add(const Fact& pFact)
{
  auto insertionResult = _facts.insert(pFact);

  if (!pFact.hasAParameter())
  {
    auto exactCallStr = _getExactCall(pFact);
    if (!_exactCallWithoutFluentToListsOpt)
      _exactCallWithoutFluentToListsOpt.emplace();
    (*_exactCallWithoutFluentToListsOpt)[exactCallStr].emplace_back(pFact);

    if (pFact.fluent())
    {
      _addFluentToExactCall(exactCallStr, pFact);
      if (!_exactCallToListsOpt)
        _exactCallToListsOpt.emplace();
      (*_exactCallToListsOpt)[exactCallStr].emplace_back(pFact);
    }
  }

  std::list<std::string> factSignatures;
  pFact.generateSignatureForAllSubTypes(factSignatures);
  for (auto& currSignature : factSignatures)
  {
    auto& factArguments = pFact.arguments();
    auto insertionRes = _signatureToLists.emplace(currSignature, factArguments.size());
    ParameterToValues& parameterToValues = insertionRes.first->second;

    parameterToValues.all.emplace_back(pFact);
    for (std::size_t i = 0; i < factArguments.size(); ++i)
    {
      if (!factArguments[i].isAParameterToFill())
        parameterToValues.argIdToArgValueToValues[i][factArguments[i].value].emplace_back(pFact);
      else
        parameterToValues.argIdToArgValueToValues[i][""].emplace_back(pFact);
    }
    if (pFact.fluent())
    {
      if (!pFact.fluent()->isAParameterToFill() && !pFact.isValueNegated())
        parameterToValues.fluentValueToValues[pFact.fluent()->value].emplace_back(pFact);
      else
        parameterToValues.fluentValueToValues[""].emplace_back(pFact);
    }
  }
}


void SetOfFact::erase(const Fact& pFact)
{
  if (!_erase(pFact))
  {
    auto factIt = find(pFact);
    for (const auto& currFact : factIt)
    {
      _erase(currFact);
      break;
    }
  }
}


bool SetOfFact::_erase(const Fact& pFact)
{
  auto it = _facts.find(pFact);
  if (it != _facts.end())
  {
    if (!pFact.hasAParameter())
    {
      auto exactCallStr = _getExactCall(pFact);
      if (_exactCallWithoutFluentToListsOpt)
      {
        std::list<Fact>& listOfTypes = (*_exactCallWithoutFluentToListsOpt)[exactCallStr];
        _removeAValueForList(listOfTypes, pFact);
        if (listOfTypes.empty())
          _exactCallWithoutFluentToListsOpt->erase(exactCallStr);
      }

      if (_exactCallToListsOpt && pFact.fluent())
      {
        _addFluentToExactCall(exactCallStr, pFact);
        std::list<Fact>& listWithFluentOfTypes = (*_exactCallToListsOpt)[exactCallStr];
        _removeAValueForList(listWithFluentOfTypes, pFact);
        if (listWithFluentOfTypes.empty())
          _exactCallToListsOpt->erase(exactCallStr);
      }
    }

    std::list<std::string> factSignatures;
    pFact.generateSignatureForAllSubTypes(factSignatures);
    for (auto& currSignature : factSignatures)
    {
      auto& factArguments = pFact.arguments();
      auto itParameterToValues = _signatureToLists.find(currSignature);
      if (itParameterToValues != _signatureToLists.end())
      {
        ParameterToValues& parameterToValues = itParameterToValues->second;

        _removeAValueForList(parameterToValues.all, pFact);
        if (parameterToValues.all.empty())
        {
          _signatureToLists.erase(currSignature);
        }
        else
        {
          for (std::size_t i = 0; i < factArguments.size(); ++i)
          {
            const std::string argKey = !factArguments[i].isAParameterToFill() ? factArguments[i].value : "";
            std::list<Fact>& listOfValues = parameterToValues.argIdToArgValueToValues[i][argKey];
            _removeAValueForList(listOfValues, pFact);
            if (listOfValues.empty())
               parameterToValues.argIdToArgValueToValues[i].erase(argKey);
          }
          if (pFact.fluent())
          {
            const std::string fluentKey = !pFact.fluent()->isAParameterToFill() ? pFact.fluent()->value : "";
            std::list<Fact>& listOfValues = parameterToValues.fluentValueToValues[fluentKey];
            _removeAValueForList(listOfValues, pFact);
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

    _facts.erase(it);
    return true;
  }
  return false;
}


std::string SetOfFact::SetOfFactIterator::toStr() const
{
  std::stringstream ss;
  ss << "[";
  bool firstElt = true;
  for (const Fact& currElt : *this)
  {
    if (firstElt)
      firstElt = false;
    else
      ss << ", ";
    ss << currElt;
  }
  ss << "]";
  return ss.str();
}


void SetOfFact::clear()
{
  _facts.clear();
  if (_exactCallToListsOpt)
    _exactCallToListsOpt.reset();
  if (_exactCallWithoutFluentToListsOpt)
    _exactCallWithoutFluentToListsOpt.reset();
  _signatureToLists.clear();
}


typename SetOfFact::SetOfFactIterator SetOfFact::find(const Fact& pFact,
                                                      bool pIgnoreFluent) const
{
  const std::list<Fact>* exactMatchPtr = nullptr;

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
    return SetOfFactIterator(exactMatchPtr);
  }

  const std::list<Fact>* resPtr = nullptr;
  auto _matchArg = [&](const std::map<std::string, std::list<Fact>>& pArgValueToValues,
                       const std::string& pArgValue) -> std::optional<typename SetOfFact::SetOfFactIterator> {
    auto itForThisValue = pArgValueToValues.find(pArgValue);
    if (itForThisValue != pArgValueToValues.end())
    {
      if (exactMatchPtr == nullptr)
      {
        if (resPtr != nullptr)
          return SetOfFactIterator(intersectTwoLists(*resPtr, itForThisValue->second));
        resPtr = &itForThisValue->second;
      }
    }
    return {};
  };

  auto itParameterToValues = _signatureToLists.find(pFact.factSignature());
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
      return SetOfFactIterator(&parameterToValues.all);
  }

  if (resPtr != nullptr)
    return SetOfFactIterator(resPtr);
  return SetOfFactIterator(exactMatchPtr);
}


void SetOfFact::_removeAValueForList(std::list<Fact>& pList,
                                     const Fact& pValue) const
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


const std::list<Fact>* SetOfFact::_findAnExactCall(
    const std::optional<std::map<std::string, std::list<Fact>>>& pExactCalls,
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

