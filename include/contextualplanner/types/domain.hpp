#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_DOMAIN_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_DOMAIN_HPP

#include <map>
#include <set>
#include "../util/api.hpp"
#include <contextualplanner/util/alias.hpp>
#include <contextualplanner/types/action.hpp>
#include <contextualplanner/types/setofinferences.hpp>

namespace cp
{

/// Set of all the actions that the bot can do with accessors to optimize the search of a action.
struct CONTEXTUALPLANNER_API Domain
{
  /// Construct an empty domain.
  Domain();

  /**
   * @brief Construct a domain.
   * @param[in] pActions Map of action identifiers to action.
   */
  Domain(const std::map<ActionId, Action>& pActions,
         const SetOfInferences& pSetOfInferences = {});

  /**
   * @brief Add an action.
   * @param pActionId[in] Identifier of the action to add.
   * @param pAction[in] Action to add.
   *
   * If the identifier is already used, the addition will not be done.
   */
  void addAction(const ActionId& pActionId,
                 const Action& pAction);

  /**
   * @brief Remove an action.
   * @param pActionId[in] Identifier of the action to remove.
   *
   * If the action is not found, this function will have no effect.<br/>
   * No exception will be raised.
   */
  void removeAction(const ActionId& pActionId);

  /// All action identifiers to action.
  const std::map<ActionId, Action>& actions() const { return _actions; }
  /// All facts in precondition to action identifiers.
  const std::map<std::string, std::set<ActionId>>& preconditionToActions() const { return _preconditionToActions; }
  /// All negationed facts in precondition to action identifiers.
  const std::map<std::string, std::set<ActionId>>& notPreconditionToActions() const { return _notPreconditionToActions; }
  /// All action identifiers of the actions without precondtion.
  const std::set<ActionId>& actionsWithoutFactToAddInPrecondition() const { return _actionsWithoutFactToAddInPrecondition; }



  // Inferences
  // ----------

  /**
   * @brief Add a set of inferences.
   * @param pSetOfInferences Set of inferences to add.
   * @param pSetOfInferencesId Identifier of the set of inferences to add.
   *
   * If the identifier is already used, the addition will not be done.
   */
  SetOfInferencesId addSetOfInferences(const SetOfInferences& pSetOfInferences,
                                       const SetOfInferencesId& pSetOfInferencesId = "soi");

  /**
   * @brief Remove a set of inferences.
   * @param pSetOfInferencesId Identifier of the set of inferences to remove.
   *
   * If the inference is not found, this function will have no effect.
   * No exception will be raised.
   */
  void removeSetOfInferences(const SetOfInferencesId& pSetOfInferencesId);

  void clearInferences();

  /// Get the set of inferences.
  const std::map<SetOfInferencesId, SetOfInferences>& getSetOfInferences() const { return _setOfInferences; }


  static const SetOfInferencesId setOfInferencesIdFromConstructor;

private:
  /// Map of action identifiers to action.
  std::map<ActionId, Action> _actions;
  /// Map of facts in precondition to action identifiers.
  std::map<std::string, std::set<ActionId>> _preconditionToActions;
  /// Map of negationed facts in precondition to action identifiers.
  std::map<std::string, std::set<ActionId>> _notPreconditionToActions;
  /// Set of action identifiers of the actions without precondtion.
  std::set<ActionId> _actionsWithoutFactToAddInPrecondition;
  /// Map set of inferences identifiers to the set of inferences.
  std::map<SetOfInferencesId, SetOfInferences> _setOfInferences;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_DOMAIN_HPP
