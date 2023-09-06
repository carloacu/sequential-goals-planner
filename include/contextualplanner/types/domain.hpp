#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_DOMAIN_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_DOMAIN_HPP

#include <map>
#include <set>
#include "../util/api.hpp"
#include <contextualplanner/util/alias.hpp>
#include <contextualplanner/types/action.hpp>


namespace cp
{

/// Set of all the actions that the bot can do.
struct CONTEXTUALPLANNER_API Domain
{
  /**
   * @brief Construct a domain.
   * @param[in] pActions Map of action identifiers to action.
   */
  Domain(const std::map<ActionId, Action>& pActions);

  /**
   * @brief Add an action.
   * @param pActionId Identifier of the action to add.
   * @param pAction Action to add.
   *
   * If the identifier is already used, the addition will not be done.
   */
  void addAction(const ActionId& pActionId,
                 const Action& pAction);

  /**
   * @brief Remove an action.
   * @param pActionId Identifier of the action to remove.
   *
   * If the action is not found, this function will have no effect.
   * No exception will be raised.
   */
  void removeAction(const ActionId& pActionId);

  /// All action identifiers to action of this domain.
  const std::map<ActionId, Action>& actions() const { return _actions; }
  /// All preconditions to action idntifiers of this domain.
  const std::map<std::string, std::set<ActionId>>& preconditionToActions() const { return _preconditionToActions; }
  /// All preconditions negationed to action idntifiers of this domain.
  const std::map<std::string, std::set<ActionId>>& notPreconditionToActions() const { return _notPreconditionToActions; }
  /// All action identifiers that does not have any fact to add in this domain.
  const std::set<ActionId>& actionsWithoutFactToAddInPrecondition() const { return _actionsWithoutFactToAddInPrecondition; }


private:
  /// Map of action identifiers to action.
  std::map<ActionId, Action> _actions;
  /// Map of preconditions to action idntifiers.
  std::map<std::string, std::set<ActionId>> _preconditionToActions;
  /// Map of preconditions negationed to action idntifiers.
  std::map<std::string, std::set<ActionId>> _notPreconditionToActions;
  /// Set of action identifiers that does not have any fact to add.
  std::set<ActionId> _actionsWithoutFactToAddInPrecondition;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_DOMAIN_HPP
