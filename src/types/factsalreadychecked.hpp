#ifndef INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_FACTSALREADYCHECKED_HPP
#define INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_FACTSALREADYCHECKED_HPP

#include <set>
#include <prioritizedgoalsplanner/types/fact.hpp>

namespace pgp
{

struct FactsAlreadyChecked
{
  std::set<Fact> factsToAdd;
  std::set<Fact> factsToRemove;

  void swap(FactsAlreadyChecked& pOther)
  {
    factsToAdd.swap(pOther.factsToAdd);
    factsToRemove.swap(pOther.factsToRemove);
  }
};


} // !pgp


#endif // INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_FACTSALREADYCHECKED_HPP
