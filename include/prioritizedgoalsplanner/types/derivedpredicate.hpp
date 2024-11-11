#ifndef INCLUDE_PRIORITIZEDGOALSPLANNER_DERIVEDPREDICATE_HPP
#define INCLUDE_PRIORITIZEDGOALSPLANNER_DERIVEDPREDICATE_HPP

#include "../util/api.hpp"
#include <prioritizedgoalsplanner/types/predicate.hpp>
#include <prioritizedgoalsplanner/types/condition.hpp>


namespace pgp
{

struct PRIORITIZEDGOALSPLANNER_API DerivedPredicate
{
  DerivedPredicate(const Predicate& pPredicate,
                   const std::string& pConditionStr,
                   const pgp::Ontology& pOntology);

  /// Copy constructor.
  DerivedPredicate(const DerivedPredicate& pDerivedPredicate);
  /// Copy operator.
  void operator=(const DerivedPredicate& pDerivedPredicate);

  /// Predicate.
  Predicate predicate;
  /// Condition to put when this is used predicate.
  std::unique_ptr<Condition> condition;
};

} // !pgp


#endif // INCLUDE_PRIORITIZEDGOALSPLANNER_DERIVEDPREDICATE_HPP
