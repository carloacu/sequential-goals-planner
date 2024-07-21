#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_ACTIONINVOCATION_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_ACTIONINVOCATION_HPP

#include <map>
#include <set>
#include <string>
#include "../util/api.hpp"


namespace cp
{
struct Entity;

/// Struct gathering the identifier of an action with some parameter values.
struct CONTEXTUALPLANNER_API ActionInvocation
{
  /// Construct ActionInstance.
  ActionInvocation(const std::string& pActionId,
                   const std::map<std::string, std::set<Entity>>& pParameters);
  ~ActionInvocation();

  /**
   * @brief Serialize in a string.
   * @return String containing the action identifier with his parameters.
   */
  std::string toStr() const;

  /// Identifer of the action.
  std::string actionId;
  /// Action parameter values.
  std::map<std::string, std::set<Entity>> parameters;
};


} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_ACTIONINVOCATION_HPP
