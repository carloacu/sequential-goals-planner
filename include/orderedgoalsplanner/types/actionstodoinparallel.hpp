#ifndef INCLUDE_ORDEREDGOALSPLANNER_TYPES_ACTIONSTODOINPARALLEL_HPP
#define INCLUDE_ORDEREDGOALSPLANNER_TYPES_ACTIONSTODOINPARALLEL_HPP

#include <list>
#include "../util/api.hpp"
#include <orderedgoalsplanner/types/actioninvocationwithgoal.hpp>


namespace ogp
{

struct ORDEREDGOALSPLANNER_API ActionsToDoInParallel
{

  std::list<ActionInvocationWithGoal> actions;
};


} // !ogp


#endif // INCLUDE_ORDEREDGOALSPLANNER_TYPES_ACTIONSTODOINPARALLEL_HPP
