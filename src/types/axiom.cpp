#include <contextualplanner/types/axiom.hpp>
#include <contextualplanner/types/event.hpp>
#include <contextualplanner/types/worldstatemodification.hpp>

namespace cp
{

Axiom::Axiom(std::unique_ptr<Condition> pCondition,
             const Fact& pFact,
             const std::vector<Parameter>& pParameters)
  : parameters(pParameters),
    condition(pCondition ? std::move(pCondition) : std::unique_ptr<Condition>()),
    fact(pFact)
{
}


std::list<Event> Axiom::toEvents(const Ontology& pOntology,
                                 const SetOfEntities& pEntities) const
{
  std::list<Event> res;
  res.emplace_back(condition->clone(), WorldStateModification::fromStr(fact.toStr(), pOntology, pEntities, parameters), parameters);
  res.emplace_back(condition->clone(nullptr, true), WorldStateModification::fromStr("!" + fact.toStr(), pOntology, pEntities, parameters), parameters);
  return res;
}


} // !cp
