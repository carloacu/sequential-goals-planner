#include <contextualplanner/types/onestepofplannerresult.hpp>


namespace cp
{

OneStepOfPlannerResult::OneStepOfPlannerResult(const std::string& pActionId,
    const std::map<std::string, std::set<std::string>>& pParameters,
    std::unique_ptr<Goal> pFromGoal,
    int pFromGoalPriority)
  : actionInvocation(pActionId, pParameters),
    fromGoal(std::move(pFromGoal)),
    fromGoalPriority(pFromGoalPriority)
{
}


OneStepOfPlannerResult::OneStepOfPlannerResult
(const OneStepOfPlannerResult& pOther)
 : actionInvocation(pOther.actionInvocation),
   fromGoal(pOther.fromGoal ? pOther.fromGoal->clone() : std::unique_ptr<cp::Goal>()),
   fromGoalPriority(pOther.fromGoalPriority)
{
}


void OneStepOfPlannerResult::operator=(const OneStepOfPlannerResult& pOther)
{
  actionInvocation = pOther.actionInvocation;
  fromGoal = pOther.fromGoal ? pOther.fromGoal->clone() : std::unique_ptr<cp::Goal>();
  fromGoalPriority = pOther.fromGoalPriority;
}


} // !cp
