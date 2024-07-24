#include <contextualplanner/types/derivedpredicate.hpp>
#include <contextualplanner/types/inference.hpp>
#include <contextualplanner/types/worldstatemodification.hpp>

namespace cp
{

DerivedPredicate::DerivedPredicate(std::unique_ptr<Condition> pCondition,
                                   const Fact& pFact,
                                   const std::vector<Parameter>& pParameters)
  : parameters(pParameters),
    condition(pCondition ? std::move(pCondition) : std::unique_ptr<Condition>()),
    fact(pFact)
{
}


std::list<Inference> DerivedPredicate::toInferences(const Ontology& pOntology,
                                                    const SetOfEntities& pEntities) const
{
  std::list<Inference> res;
  res.emplace_back(condition->clone(), WorldStateModification::fromStr(fact.toStr(), pOntology, pEntities), parameters);
  res.emplace_back(condition->clone(nullptr, true), WorldStateModification::fromStr("!" + fact.toStr(), pOntology, pEntities), parameters);
  return res;
}


} // !cp
