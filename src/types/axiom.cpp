#include <prioritizedgoalsplanner/types/axiom.hpp>
#include <prioritizedgoalsplanner/types/event.hpp>
#include <prioritizedgoalsplanner/types/worldstatemodification.hpp>
#include <prioritizedgoalsplanner/util/serializer/deserializefrompddl.hpp>

namespace cp
{

Axiom::Axiom(std::unique_ptr<Condition> pContext,
             const Fact& pImplies,
             const std::vector<Parameter>& pVars)
  : vars(pVars),
    context(pContext ? std::move(pContext) : std::unique_ptr<Condition>()),
    implies(pImplies)
{
}


std::list<Event> Axiom::toEvents(const Ontology& pOntology,
                                 const SetOfEntities& pEntities) const
{
  std::list<Event> res;
  {
    std::size_t pos = 0;
    res.emplace_back(context->clone(), pddlToWsModification(implies.toPddl(true), pos, pOntology, pEntities, vars), vars);
  }
  {
    std::size_t pos = 0;
    FactOptional impliesNegated(implies, true);
    res.emplace_back(context->clone(nullptr, true), pddlToWsModification(impliesNegated.toPddl(true), pos, pOntology, pEntities, vars), vars);
  }
  return res;
}


} // !cp
