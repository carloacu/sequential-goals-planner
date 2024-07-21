#include <contextualplanner/types/actioninvocation.hpp>
#include <contextualplanner/types/entity.hpp>

namespace cp
{

ActionInvocation::ActionInvocation(const std::string& pActionId,
    const std::map<std::string, std::set<Entity>>& pParameters)
  : actionId(pActionId),
    parameters(pParameters)
{
}

ActionInvocation::~ActionInvocation() {}


std::string ActionInvocation::toStr() const
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
        res += currParam.second.begin()->toStr();
    }
    res += ")";
  }
  return res;
}


} // !cp
