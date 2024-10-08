#ifndef INCLUDE_CONTEXTUALPLANNER_DERIVEDPREDICATE_HPP
#define INCLUDE_CONTEXTUALPLANNER_DERIVEDPREDICATE_HPP

#include "../util/api.hpp"
#include <contextualplanner/types/predicate.hpp>
#include <contextualplanner/types/condition.hpp>


namespace cp
{

struct CONTEXTUALPLANNER_API DerivedPredicate
{
  DerivedPredicate(const Predicate& pPredicate,
                   const std::string& pConditionStr,
                   const cp::Ontology& pOntology);

  /// Copy constructor.
  DerivedPredicate(const DerivedPredicate& pDerivedPredicate);
  /// Copy operator.
  void operator=(const DerivedPredicate& pDerivedPredicate);

  /// Predicate.
  Predicate predicate;
  /// Condition to put when this is used predicate.
  std::unique_ptr<Condition> condition;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_DERIVEDPREDICATE_HPP
