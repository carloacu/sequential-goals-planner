#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_ACTION_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_ACTION_HPP

#include <vector>
#include "../util/api.hpp"
#include <contextualplanner/types/condition.hpp>
#include <contextualplanner/types/problemupdate.hpp>


namespace cp
{


/// Axiomatic thing that the bot can do
struct CONTEXTUALPLANNER_API Action
{
  /**
   * @brief Construct an Action.
   * @param pPreconditions[in] Set of facts that should be present in the world to be able to do this action.
   * @param pEffect[in] How the world will change when this action will finish.
   * @param pPreferInContext[in] Set of facts that will increase the priority of this action if they are present in the world.
   */
  Action(std::unique_ptr<Condition> pPrecondition,
         const ProblemUpdate& pEffect,
         std::unique_ptr<Condition> pPreferInContext = {})
    : parameters(),
      precondition(pPrecondition ? std::move(pPrecondition) : std::unique_ptr<Condition>()),
      preferInContext(pPreferInContext ? std::move(pPreferInContext) : std::unique_ptr<Condition>()),
      effect(pEffect),
      highImportanceOfNotRepeatingIt(false)
  {
  }

  /// Move constructor of an action.
  Action(std::unique_ptr<Condition>&& pPrecondition,
         ProblemUpdate&& pEffect,
         std::unique_ptr<Condition>&& pPreferInContext)
    : parameters(),
      precondition(pPrecondition ? std::move(pPrecondition) : std::unique_ptr<Condition>()),
      preferInContext(pPreferInContext ? std::move(pPreferInContext) : std::unique_ptr<Condition>()),
      effect(std::move(pEffect)),
      highImportanceOfNotRepeatingIt(false)
  {
  }

  /// Copy constructor.
  Action(const Action& pAction)
    : parameters(pAction.parameters),
      precondition(pAction.precondition ? pAction.precondition->clone() : std::unique_ptr<Condition>()),
      preferInContext(pAction.preferInContext ? pAction.preferInContext->clone() : std::unique_ptr<Condition>()),
      effect(pAction.effect),
      highImportanceOfNotRepeatingIt(pAction.highImportanceOfNotRepeatingIt)
  {
  }

  /// Copy operator.
  void operator=(const Action& pAction)
  {
    parameters = pAction.parameters;
    precondition = pAction.precondition ? pAction.precondition->clone() : std::unique_ptr<Condition>();
    preferInContext = pAction.preferInContext ? pAction.preferInContext->clone() : std::unique_ptr<Condition>();
    effect = pAction.effect;
    highImportanceOfNotRepeatingIt = pAction.highImportanceOfNotRepeatingIt;
  }

  /// Check equality with another action.
  bool operator==(const Action& pOther) const;
  bool operator!=(const Action& pOther) const { return !operator==(pOther); }

  /// Check if this action contains a fact or the negation of the fact.
  bool hasFact(const cp::Fact& pFact) const;

  /**
   * @brief Replace a fact by another inside this action.
   * @param pOldFact[in] Current fact to replace.
   * @param pNewFact[in] New fact to set.
   */
  void replaceFact(const cp::Fact& pOldFact,
                   const cp::Fact& pNewFact);

  /// Print the precondition in string.
  std::string precondition_str() const { return precondition ? precondition->toStr() : ""; }

  /// Print preferInContext in string.
  std::string preferInContext_str() const { return preferInContext ? preferInContext->toStr() : ""; }

  /// Parameters of this action.
  std::vector<std::string> parameters;
  /// Condition that should be satisfied in the world to be able to do this action.
  std::unique_ptr<Condition> precondition;
  /// More this condition matches the world higher the priority of this action will be.
  std::unique_ptr<Condition> preferInContext;
  /// Modification to apply to the world when this action will finish.
  ProblemUpdate effect;
  /// If it is important to not repeat this action.
  bool highImportanceOfNotRepeatingIt = false;
};


} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_ACTION_HPP
