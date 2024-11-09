#include <prioritizedgoalsplanner/types/actioninvocation.hpp>
#include <prioritizedgoalsplanner/types/entity.hpp>
#include <prioritizedgoalsplanner/types/parameter.hpp>

namespace cp
{

ActionInvocation::ActionInvocation(const std::string& pActionId,
                                   const std::map<Parameter, std::set<Entity>>& pParameters)
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
      res += currParam.first.name + " -> ";
      if (!currParam.second.empty())
        res += currParam.second.begin()->value;
    }
    res += ")";
  }
  return res;
}


} // !cp
