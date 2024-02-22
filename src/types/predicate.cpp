#include <contextualplanner/types/predicate.hpp>
#include "expressionParsed.hpp"

namespace cp
{
namespace
{
void _parametersToStr(std::string& pStr,
                      const std::vector<std::string>& pParameters)
{
  bool firstIteration = true;
  for (auto& param : pParameters)
  {
    if (firstIteration)
      firstIteration = false;
    else
      pStr += ", ";
    pStr += param;
  }
}

}

Predicate::Predicate(const std::string& pStr)
  : name(),
    parameters(),
    fluent()
{
  std::size_t pos = 0;
  auto expressionParsed = ExpressionParsed::fromStr(pStr, pos);

  name = expressionParsed.name;
  for (auto& currArg : expressionParsed.arguments)
    if (currArg.followingExpression)
      parameters.emplace_back(currArg.followingExpression->name);
  if (expressionParsed.followingExpression)
    fluent = expressionParsed.followingExpression->name;
}


 std::string Predicate::toStr() const
 {
   auto res = name + "(";
   _parametersToStr(res, parameters);
   res += ")";
   if (fluent)
     res += " - " + *fluent;
   return res;
 }

} // !cp
