#include <prioritizedgoalsplanner/types/derivedpredicate.hpp>
#include <prioritizedgoalsplanner/types/setofentities.hpp>
#include <prioritizedgoalsplanner/util/serializer/deserializefrompddl.hpp>

namespace pgp
{

DerivedPredicate::DerivedPredicate(const Predicate& pPredicate,
                                   const std::string& pConditionStr,
                                   const pgp::Ontology& pOntology)
  : predicate(pPredicate),
    condition()
{
  std::vector<pgp::Parameter> parameters;
  for (const auto& currParam : pPredicate.parameters)
    if (currParam.isAParameterToFill())
      parameters.emplace_back(currParam);
  if (pPredicate.fluent)
    parameters.emplace_back(Parameter::fromType(pPredicate.fluent));

  condition = strToCondition(pConditionStr, pOntology, {}, parameters);
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


} // !pgp
