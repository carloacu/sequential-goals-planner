#include <contextualplanner/types/actioninvocationwithgoal.hpp>


namespace cp
{

ActionInvocationWithGoal::ActionInvocationWithGoal(const std::string& pActionId,
    const std::map<std::string, std::set<Entity>>& pParameters,
    std::unique_ptr<Goal> pFromGoal,
    int pFromGoalPriority)
  : actionInvocation(pActionId, pParameters),
    fromGoal(std::move(pFromGoal)),
    fromGoalPriority(pFromGoalPriority)
{
}


ActionInvocationWithGoal::ActionInvocationWithGoal
(const ActionInvocationWithGoal& pOther)
 : actionInvocation(pOther.actionInvocation),
   fromGoal(pOther.fromGoal ? pOther.fromGoal->clone() : std::unique_ptr<cp::Goal>()),
   fromGoalPriority(pOther.fromGoalPriority)
{
}


void ActionInvocationWithGoal::operator=(const ActionInvocationWithGoal& pOther)
{
  actionInvocation = pOther.actionInvocation;
  fromGoal = pOther.fromGoal ? pOther.fromGoal->clone() : std::unique_ptr<cp::Goal>();
  fromGoalPriority = pOther.fromGoalPriority;
}


} // !cp
