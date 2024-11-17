#include <prioritizedgoalsplanner/types/actioninvocation.hpp>
#include <prioritizedgoalsplanner/types/entity.hpp>
#include <prioritizedgoalsplanner/types/parameter.hpp>

namespace pgp
{

ActionInvocation::ActionInvocation(const std::string& pActionId,
                                   const std::map<Parameter, Entity>& pParameters)
  : actionId(pActionId),
    parameters(pParameters)
{
}


ActionInvocation::ActionInvocation(const std::string& pActionId,
                                   const std::map<Parameter, std::set<Entity>>& pParameters)
  : actionId(pActionId),
    parameters()
{
  for (auto& currParam : pParameters)
    if (!currParam.second.empty())
      parameters.emplace(currParam.first, *currParam.second.begin());
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
      res += currParam.first.name + " -> " + currParam.second.value;
    }
    res += ")";
  }
  return res;
}


} // !pgp
