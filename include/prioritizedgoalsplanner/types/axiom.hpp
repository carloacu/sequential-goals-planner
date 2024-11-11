#ifndef INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_AXIOM_HPP
#define INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_AXIOM_HPP

#include <assert.h>
#include <list>
#include <vector>
#include <prioritizedgoalsplanner/types/condition.hpp>
#include <prioritizedgoalsplanner/types/fact.hpp>

namespace pgp
{
struct Event;


/// Specification what is a derived predicate.
struct PRIORITIZEDGOALSPLANNER_API Axiom
{
  /// Construct a derived predicate.
  Axiom(std::unique_ptr<Condition> pContext,
        const Fact& pImplies,
        const std::vector<Parameter>& pVars = {});

  /// Construct a copy.
  Axiom(const Axiom& pOther)
    : vars(pOther.vars),
      context(pOther.context ? pOther.context->clone() : std::unique_ptr<Condition>()),
      implies(pOther.implies)
  {
    assert(context);
  }

  /// Convert this derived predicate to 2 events.
  std::list<Event> toEvents(const Ontology& pOntology,
                            const SetOfEntities& pEntities) const;

  /// Parameter names of this derived predicate.
  std::vector<Parameter> vars;
  /**
   * Condition to apply.
   */
  const std::unique_ptr<Condition> context;

  /// Fact produced by the derived predicate.
  const Fact implies;
};



} // !pgp


#endif // INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_AXIOM_HPP
