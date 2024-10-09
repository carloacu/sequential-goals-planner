#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_AXIOM_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_AXIOM_HPP

#include <assert.h>
#include <list>
#include <vector>
#include <contextualplanner/types/condition.hpp>
#include <contextualplanner/types/fact.hpp>

namespace cp
{
struct Event;


/// Specification what is a derived predicate.
struct CONTEXTUALPLANNER_API Axiom
{
  /// Construct a derived predicate.
  Axiom(std::unique_ptr<Condition> pCondition,
        const Fact& pFact,
        const std::vector<Parameter>& pParameters = {});

  /// Construct a copy.
  Axiom(const Axiom& pOther)
    : parameters(pOther.parameters),
      condition(pOther.condition ? pOther.condition->clone() : std::unique_ptr<Condition>()),
      fact(pOther.fact)
  {
    assert(condition);
  }

  /// Convert this derived predicate to 2 events.
  std::list<Event> toEvents(const Ontology& pOntology,
                            const SetOfEntities& pEntities) const;

  /// Parameter names of this derived predicate.
  std::vector<Parameter> parameters;
  /**
   * Condition to apply the facts and goals modification.
   * The condition is true if the condition is a sub set of a corresponding world state.
   */
  const std::unique_ptr<Condition> condition;

  /// Fact produced by the derived predicate.
  const Fact fact;
};



} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_AXIOM_HPP
