#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_DERIVEDPREDICATE_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_DERIVEDPREDICATE_HPP

#include <assert.h>
#include <list>
#include <vector>
#include <contextualplanner/types/condition.hpp>
#include <contextualplanner/types/fact.hpp>

namespace cp
{
struct Inference;


/// Specification what is a derived predicate.
struct CONTEXTUALPLANNER_API DerivedPredicate
{
  /// Construct a derived predicate.
  DerivedPredicate(std::unique_ptr<Condition> pCondition,
                   const Fact& pFact,
                   const std::vector<std::string>& pParameters = {});

  /// Construct a copy.
  DerivedPredicate(const DerivedPredicate& pOther)
    : parameters(pOther.parameters),
      condition(pOther.condition ? pOther.condition->clone() : std::unique_ptr<Condition>()),
      fact(pOther.fact)
  {
    assert(condition);
  }

  /// Convert this derived predicate to 2 inferences.
  std::list<Inference> toInferences() const;

  /// Parameter names of this derived predicate.
  std::vector<std::string> parameters;
  /**
   * Condition to apply the facts and goals modification.
   * The condition is true if the condition is a sub set of a corresponding world state.
   */
  const std::unique_ptr<Condition> condition;

  /// Fact produced by the derived predicate.
  const Fact fact;
};



} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_DERIVEDPREDICATE_HPP
