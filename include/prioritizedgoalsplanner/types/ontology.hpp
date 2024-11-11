#ifndef INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_ONTOLOGY_HPP
#define INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_ONTOLOGY_HPP

#include "../util/api.hpp"
#include "setofderivedpredicates.hpp"
#include "setofentities.hpp"
#include "setofpredicates.hpp"
#include "setoftypes.hpp"


namespace pgp
{

struct PRIORITIZEDGOALSPLANNER_API Ontology
{
  SetOfTypes types;
  SetOfPredicates predicates;
  SetOfEntities constants;
  SetOfDerivedPredicates derivedPredicates;
};

} // namespace pgp

#endif // INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_ONTOLOGY_HPP
