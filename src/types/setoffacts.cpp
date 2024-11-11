#include <prioritizedgoalsplanner/types/setoffacts.hpp>
#include <stdexcept>
#include <prioritizedgoalsplanner/types/fact.hpp>
#include <prioritizedgoalsplanner/util/alias.hpp>
#include <prioritizedgoalsplanner/util/util.hpp>
#include "expressionParsed.hpp"

namespace pgp
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



SetOfFacts::SetOfFacts()
 : _facts(),
   _exactCallToListsOpt(),
   _exactCallWithoutFluentToListsOpt(),
   _signatureToLists()
{
}


SetOfFacts SetOfFacts::fromPddl(const std::string& pStr,
                                std::size_t& pPos,
                                const Ontology& pOntology,
                                const SetOfEntities& pEntities,
                                bool pCanFactsBeRemoved)
{
  SetOfFacts res;
  res.addFactsFromPddl(pStr, pPos, pOntology, pEntities, pCanFactsBeRemoved);
  return res;
}


std::string SetOfFacts::toPddl(std::size_t pIdentation, bool pPrintTimeLessFactsToo) const
{
  std::string res;
  bool firstIteration = true;
  for (auto& currFact : _facts)
  {
    if (!pPrintTimeLessFactsToo && !currFact.second)
      continue;
    if (firstIteration)
      firstIteration = false;
    else
      res += "\n";
    res += std::string(pIdentation, ' ') + currFact.first.toPddl(false, true);
  }
  return res;
}


void SetOfFacts::addFactsFromPddl(const std::string& pStr,
                                  std::size_t& pPos,
                                  const Ontology& pOntology,
                                  const SetOfEntities& pEntities,
                                  bool pCanFactsBeRemoved)
{
  auto strSize = pStr.size();
  ExpressionParsed::skipSpaces(pStr, pPos);

  while (pPos < strSize && pStr[pPos] != ')')
    add(Fact(pStr, true, pOntology, pEntities, {}, nullptr, pPos, &pPos), pCanFactsBeRemoved);
}


void SetOfFacts::add(const Fact& pFact,
                     bool pCanBeRemoved)
{
  auto insertionResult = _facts.emplace(pFact, pCanBeRemoved);

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


bool SetOfFacts::erase(const Fact& pFact)
{
  if (_erase(pFact))
    return true;

  auto factIt = find(pFact);
  for (const auto& currFact : factIt)
    return _erase(currFact);
  return false;
}


bool SetOfFacts::_erase(const Fact& pFact)
{
  auto it = _facts.find(pFact);
  if (it != _facts.end())
  {
    // Do not allow to remove if this fact is marked as cannot be removed
    if (!it->second)
      return false;

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


std::string SetOfFacts::SetOfFactIterator::toStr() const
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


void SetOfFacts::clear()
{
  _facts.clear();
  if (_exactCallToListsOpt)
    _exactCallToListsOpt.reset();
  if (_exactCallWithoutFluentToListsOpt)
    _exactCallWithoutFluentToListsOpt.reset();
  _signatureToLists.clear();
}


typename SetOfFacts::SetOfFactIterator SetOfFacts::find(const Fact& pFact,
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
                       const std::string& pArgValue) -> std::optional<typename SetOfFacts::SetOfFactIterator> {
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

std::optional<Entity> SetOfFacts::getFactFluent(const pgp::Fact& pFact) const
{
  auto factMatchingInWs = find(pFact, true);
  for (const auto& currFact : factMatchingInWs)
    if (currFact.arguments() == pFact.arguments())
      return currFact.fluent();
  return {};
}


void SetOfFacts::extractPotentialArgumentsOfAFactParameter(
    std::set<Entity>& pPotentialArgumentsOfTheParameter,
    const Fact& pFact,
    const std::string& pParameter) const
{
  auto factMatchingInWs = find(pFact);
  for (const auto& currFact : factMatchingInWs)
  {
    if (currFact.arguments().size() == pFact.arguments().size())
    {
      std::set<Entity> potentialNewValues;
      bool doesItMatch = true;
      for (auto i = 0; i < pFact.arguments().size(); ++i)
      {
        if (pFact.arguments()[i].value == pParameter)
        {
          potentialNewValues.insert(currFact.arguments()[i]);
          continue;
        }
        if (pFact.arguments()[i] == currFact.arguments()[i])
          continue;
        doesItMatch = false;
        break;
      }
      if (doesItMatch)
      {
        if (pPotentialArgumentsOfTheParameter.empty())
          pPotentialArgumentsOfTheParameter = std::move(potentialNewValues);
        else
          pPotentialArgumentsOfTheParameter.insert(potentialNewValues.begin(), potentialNewValues.end());
      }
    }
  }
}


void SetOfFacts::_removeAValueForList(std::list<Fact>& pList,
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


const std::list<Fact>* SetOfFacts::_findAnExactCall(
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



} // !pgp

