#ifndef INCLUDE_CONTEXTUALPLANNER_DOMAIN_HPP
#define INCLUDE_CONTEXTUALPLANNER_DOMAIN_HPP

#include <map>
#include "api.hpp"
#include <contextualplanner/alias.hpp>
#include <contextualplanner/action.hpp>



namespace cp
{


struct CONTEXTUALPLANNER_API Domain
{
  Domain(const std::map<ActionId, Action>& pActions);

  void addAction(ActionId pActionId,
                 const Action& pAction);
  void removeAction(ActionId pActionId);

  const std::map<ActionId, Action>& actions() const { return _actions; }
  const std::map<std::string, std::set<ActionId>>& preconditionToActions() const { return _preconditionToActions; }
  const std::map<std::string, std::set<ActionId>>& preconditionToActionsExps() const { return _preconditionToActionsExps; }
  const std::map<std::string, std::set<ActionId>>& notPreconditionToActions() const { return _notPreconditionToActions; }
  const std::set<ActionId>& actionsWithoutPrecondition() const { return _actionsWithoutPrecondition; }

private:
  std::map<ActionId, Action> _actions;
  std::map<std::string, std::set<ActionId>> _preconditionToActions;
  std::map<std::string, std::set<ActionId>> _preconditionToActionsExps;
  std::map<std::string, std::set<ActionId>> _notPreconditionToActions;
  std::set<ActionId> _actionsWithoutPrecondition;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_DOMAIN_HPP
