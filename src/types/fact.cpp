#include <contextualplanner/types/fact.hpp>
#include <contextualplanner/types/factoptional.hpp>
#include <assert.h>


namespace cp
{
namespace {

void _parametersToStr(std::string& pStr,
                      const std::vector<FactOptional>& pParameters)
{
  bool firstIteration = true;
  for (auto& param : pParameters)
  {
    if (firstIteration)
      firstIteration = false;
    else
      pStr += ", ";
    pStr += param.toStr();
  }
}
}

const std::string Fact::anyValue = "<any>_it_is_a_language_token_for_the_planner_engine";
const std::string Fact::punctualPrefix = "_punctual_";

Fact::Fact(const std::string& pName)
  : name(pName),
    parameters(),
    value()
{
}

Fact::Fact(const std::string& pStr,
           std::size_t pBeginPos,
           char pSeparator,
           bool* pIsFactNegatedPtr,
           std::size_t* pResPos)
  : name(),
    parameters(),
    value()
{
  auto resPos = fillFactFromStr(pStr, pBeginPos, pSeparator, pIsFactNegatedPtr);
  if (pResPos != nullptr)
    *pResPos = resPos;
}

Fact::~Fact()
{
}

bool Fact::operator<(const Fact& pOther) const
{
  if (name != pOther.name)
    return name < pOther.name;
  if (value != pOther.value)
    return value < pOther.value;
  std::string paramStr;
  _parametersToStr(paramStr, parameters);
  std::string otherParamStr;
  _parametersToStr(otherParamStr, pOther.parameters);
  return paramStr < otherParamStr;
}

bool Fact::operator==(const Fact& pOther) const
{
  return name == pOther.name && value == pOther.value && parameters == pOther.parameters;
}

bool Fact::isPunctual() const
{
  return name.compare(0, punctualPrefix.size(), punctualPrefix) == 0;
}

bool Fact::areEqualExceptAnyValues(const Fact& pOther) const
{
  if (!(name == pOther.name &&
        (value == pOther.value || value == anyValue || pOther.value == anyValue) &&
        parameters.size() == pOther.parameters.size()))
    return false;

  auto itParam = parameters.begin();
  auto itOtherParam = parameters.begin();
  while (itParam != parameters.end())
  {
    if (*itParam != *itOtherParam && *itParam != anyValue && *itOtherParam != anyValue)
      return false;
    ++itParam;
    ++itOtherParam;
  }
  return true;
}


std::string Fact::tryToExtractParameterValueFromExemple(
    const std::string& pParameterValue,
    const Fact& pOther) const
{
  if (name != pOther.name ||
      parameters.size() != pOther.parameters.size())
    return "";

  std::string res;
  if (value != pOther.value)
  {
    if (value == pParameterValue)
      res = pOther.value;
    else
      return "";
  }

  auto itParam = parameters.begin();
  auto itOtherParam = pOther.parameters.begin();
  while (itParam != parameters.end())
  {
    auto subRes = itParam->fact.tryToExtractParameterValueFromExemple(pParameterValue, itOtherParam->fact);
    if (subRes != "")
      return subRes;
    ++itParam;
    ++itOtherParam;
  }
  return res;
}


void Fact::fillParameters(
    const std::map<std::string, std::string>& pParameters)
{
  auto itValueParam = pParameters.find(value);
  if (itValueParam != pParameters.end())
    value = itValueParam->second;

  for (auto& currParam : parameters)
  {
    auto& currFactParam = currParam.fact;
    if (currFactParam.value.empty() && currFactParam.parameters.empty())
    {
      auto itValueParam = pParameters.find(currFactParam.name);
      if (itValueParam != pParameters.end())
        currFactParam.name = itValueParam->second;
    }
    else
    {
      currFactParam.fillParameters(pParameters);
    }
  }
}



std::string Fact::toStr() const
{
  std::string res = name;
  if (!parameters.empty())
  {
    res += "(";
    _parametersToStr(res, parameters);
    res += ")";
  }
  if (!value.empty())
    res += "=" + value;
  return res;
}


Fact Fact::fromStr(const std::string& pStr,
                   bool* pIsFactNegatedPtr)
{
  Fact res;
  auto endPos = res.fillFactFromStr(pStr, 0, ',', pIsFactNegatedPtr);
  assert(!res.name.empty());
  assert(endPos == pStr.size());
  return res;
}


std::size_t Fact::fillFactFromStr(
    const std::string& pStr,
    std::size_t pBeginPos,
    char pSeparator,
    bool* pIsFactNegatedPtr)
{
  std::size_t pos = pBeginPos;
  while (pos < pStr.size())
  {
    if (pStr[pos] == ' ')
    {
      ++pos;
      continue;
    }
    if (pStr[pos] == '!')
    {
      if (pIsFactNegatedPtr != nullptr)
        *pIsFactNegatedPtr = true;
      ++pos;
      continue;
    }
    if (pStr[pos] == pSeparator || pStr[pos] == ')')
      return pos;

    bool insideParenthesis = false;
    auto beginPos = pos;
    while (pos < pStr.size())
    {
      if (!insideParenthesis && (pStr[pos] == pSeparator || pStr[pos] == ' ' || pStr[pos] == ')'))
        break;
      if (pStr[pos] == '(' || pStr[pos] == ',')
      {
        insideParenthesis = true;
        if (name.empty())
          name = pStr.substr(beginPos, pos - beginPos);
        ++pos;
        parameters.emplace_back(pStr, pos, ',', &pos);
        beginPos = pos;
        continue;
      }
      if (pStr[pos] == ')' || pStr[pos] == '=')
      {
        insideParenthesis = false;
        if (name.empty())
          name = pStr.substr(beginPos, pos - beginPos);
        ++pos;
        beginPos = pos;
        continue;
      }
      ++pos;
    }
    if (name.empty())
      name = pStr.substr(beginPos, pos - beginPos);
    else
      value = pStr.substr(beginPos, pos - beginPos);
  }
  return pos;
}


bool Fact::replaceParametersByAny(const std::vector<std::string>& pParameters)
{
  bool res = false;
  for (const auto& currParam : pParameters)
  {
    for (auto& currFactParam : parameters)
    {
      if (currFactParam == currParam)
      {
        currFactParam = anyValue;
        res = true;
      }
    }
    if (value == currParam)
    {
      value = anyValue;
      res = true;
    }
  }
  return res;
}


bool Fact::isInFacts(
    const std::set<Fact>& pFacts,
    bool pParametersAreForTheFact,
    std::map<std::string, std::string>* pParametersPtr) const
{
  for (const auto& currFact : pFacts)
  {
    if (currFact.name != name ||
        currFact.parameters.size() != parameters.size())
      continue;

    auto doesItMatch = [&](const std::string& pFactValue, const std::string& pValueToLookFor) {
      if (pFactValue == pValueToLookFor)
        return true;
      if (pParametersPtr == nullptr || pParametersPtr->empty())
        return false;
      auto& parameters = *pParametersPtr;

      auto itParam = parameters.find(pFactValue);
      if (itParam != parameters.end())
      {
        if (!itParam->second.empty())
        {
          if (itParam->second == pValueToLookFor)
            return true;
        }
        else
        {
          parameters[pFactValue] = pValueToLookFor;
          return true;
        }
      }
      return false;
    };

    {
      bool doesParametersMatches = true;
      auto itFactParameters = currFact.parameters.begin();
      auto itLookForParameters = parameters.begin();
      while (itFactParameters != currFact.parameters.end())
      {
        if (*itFactParameters != *itLookForParameters)
        {
          if (!itFactParameters->fact.parameters.empty() ||
              !itFactParameters->fact.value.empty() ||
              !itLookForParameters->fact.parameters.empty() ||
              !itLookForParameters->fact.value.empty() ||
              (!pParametersAreForTheFact && !doesItMatch(itFactParameters->fact.name, itLookForParameters->fact.name)) ||
              (pParametersAreForTheFact && !doesItMatch(itLookForParameters->fact.name, itFactParameters->fact.name)))
            doesParametersMatches = false;
        }
        ++itFactParameters;
        ++itLookForParameters;
      }
      if (!doesParametersMatches)
        continue;
    }

    if (pParametersAreForTheFact)
    {
      if (doesItMatch(value, currFact.value))
        return true;
    }
    else
    {
      if (doesItMatch(currFact.value, value))
        return true;
    }
  }
  return false;
}


} // !cp
