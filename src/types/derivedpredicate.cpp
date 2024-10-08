#include <contextualplanner/types/derivedpredicate.hpp>
#include <contextualplanner/types/setofentities.hpp>

namespace cp
{

DerivedPredicate::DerivedPredicate(const Predicate& pPredicate,
                                   const std::string& pConditionStr,
                                   const cp::Ontology& pOntology)
  : predicate(pPredicate),
    condition()
{
  std::vector<cp::Parameter> parameters;
  for (const auto& currParam : pPredicate.parameters)
    if (currParam.isAParameterToFill())
      parameters.emplace_back(currParam);
  if (pPredicate.fluent)
    parameters.emplace_back(Parameter::fromType(pPredicate.fluent));

  condition = cp::Condition::fromStr(pConditionStr, pOntology, {}, parameters);
}


DerivedPredicate::DerivedPredicate(const DerivedPredicate& pDerivedPredicate)
  : predicate(pDerivedPredicate.predicate),
    condition(pDerivedPredicate.condition->clone())
{
}

void DerivedPredicate::operator=(const DerivedPredicate& pDerivedPredicate)
{
  predicate = pDerivedPredicate.predicate;
  condition = pDerivedPredicate.condition->clone();
}


} // !cp
