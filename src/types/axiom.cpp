#include <contextualplanner/types/axiom.hpp>
#include <contextualplanner/types/event.hpp>
#include <contextualplanner/types/worldstatemodification.hpp>

namespace cp
{

Axiom::Axiom(std::unique_ptr<Condition> pImplies,
             const Fact& pContext,
             const std::vector<Parameter>& pVars)
  : vars(pVars),
    context(pImplies ? std::move(pImplies) : std::unique_ptr<Condition>()),
    implies(pContext)
{
}


std::list<Event> Axiom::toEvents(const Ontology& pOntology,
                                 const SetOfEntities& pEntities) const
{
  std::list<Event> res;
  res.emplace_back(context->clone(), WorldStateModification::fromStr(implies.toStr(), pOntology, pEntities, vars), vars);
  res.emplace_back(context->clone(nullptr, true), WorldStateModification::fromStr("!" + implies.toStr(), pOntology, pEntities, vars), vars);
  return res;
}


} // !cp
