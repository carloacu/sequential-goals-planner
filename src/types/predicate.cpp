#include <contextualplanner/types/predicate.hpp>
#include <contextualplanner/types/setoftypes.hpp>
#include "expressionParsed.hpp"


namespace cp
{
namespace
{
void _parametersToStr(std::string& pStr,
                      const std::vector<Parameter>& pParameters)
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

Predicate::Predicate(const std::string& pStr,
                     const SetOfTypes& pSetOfTypes)
  : name(),
    parameters(),
    fluent()
{
  std::size_t pos = 0;
  auto expressionParsed = ExpressionParsed::fromStr(pStr, pos);

  name = expressionParsed.name;
  for (auto& currArg : expressionParsed.arguments)
    if (currArg.followingExpression)
    {
      auto type = pSetOfTypes.nameToType(currArg.followingExpression->name);
      parameters.emplace_back(Parameter(currArg.name, type));
    }
  if (expressionParsed.followingExpression) {
    fluent = pSetOfTypes.nameToType(expressionParsed.followingExpression->name);
  }
}


 std::string Predicate::toStr() const
 {
   auto res = name + "(";
   _parametersToStr(res, parameters);
   res += ")";
   if (fluent)
     res += " - " + fluent->name;
   return res;
 }

} // !cp
