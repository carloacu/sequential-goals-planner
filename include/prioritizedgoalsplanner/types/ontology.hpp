#ifndef INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_ONTOLOGY_HPP
#define INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_ONTOLOGY_HPP

#include "../util/api.hpp"
#include "setofderivedpredicates.hpp"
#include "setofentities.hpp"
#include "setofpredicates.hpp"
#include "setoftypes.hpp"


namespace cp
{

struct PRIORITIZEDGOALSPLANNER_API Ontology
{
  SetOfTypes types;
  SetOfPredicates predicates;
  SetOfEntities constants;
  SetOfDerivedPredicates derivedPredicates;
};

} // namespace cp

#endif // INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_ONTOLOGY_HPP
