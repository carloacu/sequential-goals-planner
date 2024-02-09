#include "expressionParsed.hpp"
#include <stdexcept>
#include <contextualplanner/types/factoptional.hpp>

namespace cp
{

namespace
{

bool _isASeparatorForTheBeginOfAFollowingExpression(char pChar)
{
  return pChar == '&' || pChar == '|' || pChar == '+' || pChar == '-' || pChar == '<' || pChar == '>';
}

bool _isASeparator(char pChar)
{
  return pChar == ' ' || pChar == '(' || pChar == ')' || pChar == ',' || pChar == '=' ||
      _isASeparatorForTheBeginOfAFollowingExpression(pChar);
}


}



FactOptional ExpressionParsed::toFact() const
{
  FactOptional res(name);
  for (auto& currArg : arguments)
    res.fact.arguments.emplace_back(currArg.toFact());
  if (value == Fact::undefinedValue && !res.isFactNegated)
  {
    res.isFactNegated = true;
    res.fact.value = Fact::anyValue;
  }
  else
  {
    res.fact.value = value;
  }
  return res;
}


ExpressionParsed ExpressionParsed::fromStr(const std::string& pStr,
                                           std::size_t& pPos)
{
  ExpressionParsed res;

  auto strSize = pStr.size();
  std::size_t beginOfNamePos = pPos;

  // extract name
  while (pPos < strSize)
  {
    if (beginOfNamePos == pPos)
    {
      if (pStr[pPos] == ' ')
      {
        ++pPos;
        beginOfNamePos = pPos;
        continue;
      }
    }
    else
    {
      if (_isASeparator(pStr[pPos]))
      {
        res.name = pStr.substr(beginOfNamePos, pPos - beginOfNamePos);
        break;
      }
    }
    ++pPos;
  }

  if (res.name.empty())
  {
    if (beginOfNamePos == pPos)
      throw std::runtime_error("Predicate is missing in expression: \"" + pStr + "\"");
    res.name = pStr.substr(beginOfNamePos, pPos - beginOfNamePos);
  }

  // extract arguments
  if (pPos < strSize)
  {
    if (pStr[pPos] == '(')
    {
      res.isAFunction = true;
      if (pPos + 1 < strSize && pStr[pPos + 1] != ')')
      {
        do
        {
          ++pPos;
          res.arguments.emplace_back(fromStr(pStr, pPos));
        }
        while (pStr[pPos] == ',');
      }
      else
      {
        ++pPos;
      }

      if (pStr[pPos] == ')')
        ++pPos;
      else
        throw std::runtime_error("Arguments parenthesis is not closed: \"" + pStr + "\"");
    }
  }

  // extract value
  if (pPos < strSize)
  {
    if (pStr[pPos] == '=')
    {
      res.isAFunction = true;
      ++pPos;
      std::size_t beginOfValuePos = pPos;

      while (pPos < strSize)
      {
        if (_isASeparator(pStr[pPos]))
        {
          res.value = pStr.substr(beginOfValuePos, pPos - beginOfValuePos);
          break;
        }
        ++pPos;
      }

      if (res.value.empty() && pPos > beginOfValuePos)
        res.value = pStr.substr(beginOfValuePos, pPos - beginOfValuePos);
    }
  }

  // extract following expression
  while (pPos < strSize)
  {
    if (pStr[pPos] == ' ')
    {
      ++pPos;
      continue;
    }

    if (_isASeparatorForTheBeginOfAFollowingExpression(pStr[pPos]))
    {
      res.separatorToFollowingExp = pStr[pPos];
      ++pPos;
      res.followingExpression = std::make_unique<ExpressionParsed>(fromStr(pStr, pPos));
    }

    return res;
  }

  return res;
}



} // !cp
