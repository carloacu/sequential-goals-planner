#include <contextualplanner/types/onestepofplannerresult.hpp>


namespace cp
{

ActionInstance::ActionInstance(const std::string& pActionId,
    const std::map<std::string, std::set<std::string>>& pParameters)
  : actionId(pActionId),
    parameters(pParameters)
{
}

std::string ActionInstance::toStr() const
{
  std::string res = actionId;
  if (!parameters.empty())
  {
    res += "(";
    bool firstIeration = true;
    for (const auto& currParam : parameters)
    {
      if (firstIeration)
        firstIeration = false;
      else
        res += ", ";
      res += currParam.first + " -> ";
      if (!currParam.second.empty())
        res += *currParam.second.begin();
    }
    res += ")";
  }
  return res;
}


OneStepOfPlannerResult::OneStepOfPlannerResult(const std::string& pActionId,
    const std::map<std::string, std::set<std::string>>& pParameters,
    std::unique_ptr<Goal> pFromGoal,
    int pFromGoalPriority)
  : actionInstance(pActionId, pParameters),
    fromGoal(std::move(pFromGoal)),
    fromGoalPriority(pFromGoalPriority)
{
}


OneStepOfPlannerResult::OneStepOfPlannerResult
(const OneStepOfPlannerResult& pOther)
 : actionInstance(pOther.actionInstance),
   fromGoal(pOther.fromGoal ? pOther.fromGoal->clone() : std::unique_ptr<cp::Goal>()),
   fromGoalPriority(pOther.fromGoalPriority)
{
}


void OneStepOfPlannerResult::operator=(const OneStepOfPlannerResult& pOther)
{
  actionInstance = pOther.actionInstance;
  fromGoal = pOther.fromGoal ? pOther.fromGoal->clone() : std::unique_ptr<cp::Goal>();
  fromGoalPriority = pOther.fromGoalPriority;
}


} // !cp
