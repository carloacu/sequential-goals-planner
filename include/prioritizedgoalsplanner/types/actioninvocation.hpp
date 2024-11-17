#ifndef INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_ACTIONINVOCATION_HPP
#define INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_ACTIONINVOCATION_HPP

#include <map>
#include <set>
#include <string>
#include "../util/api.hpp"


namespace pgp
{
struct Entity;
struct Parameter;


/// Struct gathering the identifier of an action with some parameter values.
struct PRIORITIZEDGOALSPLANNER_API ActionInvocation
{
  /// Construct ActionInstance.
  ActionInvocation(const std::string& pActionId,
                   const std::map<Parameter, Entity>& pParameters);
  ActionInvocation(const std::string& pActionId,
                   const std::map<Parameter, std::set<Entity>>& pParameters);
  ~ActionInvocation();

  /**
   * @brief Serialize in a string.
   * @return String containing the action identifier with his parameters.
   */
  std::string toStr() const;

  /// Identifer of the action.
  std::string actionId;
  /// Action parameter values.
  std::map<Parameter, Entity> parameters;
};


} // !pgp


#endif // INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_ACTIONINVOCATION_HPP
