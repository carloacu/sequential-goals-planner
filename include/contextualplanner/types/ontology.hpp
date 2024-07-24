#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_ONTOLOGY_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_ONTOLOGY_HPP

#include "../util/api.hpp"
#include "setofentities.hpp"
#include "setofpredicates.hpp"
#include "setoftypes.hpp"


namespace cp
{

struct CONTEXTUALPLANNER_API Ontology
{
  SetOfTypes types;
  SetOfPredicates predicates;
  SetOfEntities constants;

  bool empty() const;
};

} // namespace cp

#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_ONTOLOGY_HPP
