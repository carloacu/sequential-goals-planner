#include <contextualplanner/types/onestepofplannerresult.hpp>


namespace cp
{

ActionInstance::ActionInstance(
    const std::string& pActionId,
    const std::map<std::string, std::string>& pParameters)
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
      res += currParam.first + " -> " + currParam.second;
    }
    res += ")";
  }
  return res;
}


OneStepOfPlannerResult::OneStepOfPlannerResult(
    const std::string& pActionId,
    const std::map<std::string, std::string>& pParameters,
    const cp::Goal& pFromGoal,
    int pFromGoalPriority)
  : actionInstance(pActionId, pParameters),
    fromGoal(pFromGoal),
    fromGoalPriority(pFromGoalPriority)
{
}

} // !cp
