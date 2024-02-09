#ifndef INCLUDE_CONTEXTUALPLANNER_EXPRESSION_PARSED_HPP
#define INCLUDE_CONTEXTUALPLANNER_EXPRESSION_PARSED_HPP

#include <list>
#include <memory>
#include <string>

namespace cp
{
struct FactOptional;


struct ExpressionParsed
{

  std::string name;
  std::list<ExpressionParsed> arguments;
  std::string value;
  bool isAFunction = false;

  char separatorToFollowingExp;
  std::unique_ptr<ExpressionParsed> followingExpression;

  FactOptional toFact() const;

  static ExpressionParsed fromStr(const std::string& pStr,
                                  std::size_t& pPos);
};


} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_EXPRESSION_PARSED_HPP
