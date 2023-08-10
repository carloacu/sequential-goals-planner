#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_FACTSALREADYCHECKED_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_FACTSALREADYCHECKED_HPP

#include <set>
#include "../util/api.hpp"
#include <contextualplanner/types/fact.hpp>

namespace cp
{

struct FactsAlreadyChecked
{
  std::set<Fact> factsToAdd;
  std::set<Fact> factsToRemove;
};


} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_FACTSALREADYCHECKED_HPP