#ifndef INCLUDE_ORDEREDGOALSPLANNER_TYPES_FACTSALREADYCHECKED_HPP
#define INCLUDE_ORDEREDGOALSPLANNER_TYPES_FACTSALREADYCHECKED_HPP

#include <set>
#include <orderedgoalsplanner/types/fact.hpp>

namespace ogp
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


} // !ogp


#endif // INCLUDE_ORDEREDGOALSPLANNER_TYPES_FACTSALREADYCHECKED_HPP
