#ifndef INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_ACTIONSTODOINPARALLEL_HPP
#define INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_ACTIONSTODOINPARALLEL_HPP

#include <list>
#include "../util/api.hpp"
#include <prioritizedgoalsplanner/types/actioninvocationwithgoal.hpp>


namespace pgp
{

struct PRIORITIZEDGOALSPLANNER_API ActionsToDoInParallel
{

  std::list<ActionInvocationWithGoal> actions;
};


} // !pgp


#endif // INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_ACTIONSTODOINPARALLEL_HPP
