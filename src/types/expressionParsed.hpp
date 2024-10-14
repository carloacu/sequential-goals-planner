#ifndef INCLUDE_CONTEXTUALPLANNER_EXPRESSION_PARSED_HPP
#define INCLUDE_CONTEXTUALPLANNER_EXPRESSION_PARSED_HPP

#include <list>
#include <memory>
#include <string>
#include <vector>

namespace cp
{
struct FactOptional;
struct Ontology;
struct Parameter;
struct SetOfEntities;


struct ExpressionParsed
{

  std::string name;
  std::list<ExpressionParsed> arguments;
  std::string value;
  bool isValueNegated = false;
  bool isAFunction = false;

  char separatorToFollowingExp;
  std::unique_ptr<ExpressionParsed> followingExpression;

  FactOptional toFact(const Ontology& pOntology,
                      const SetOfEntities& pEntities,
                      const std::vector<Parameter>& pParameters,
                      bool pIsOkIfFluentIsMissing) const;

  static ExpressionParsed fromStr(const std::string& pStr,
                                  std::size_t& pPos);
};


} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_EXPRESSION_PARSED_HPP
