#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_ACTION_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_ACTION_HPP

#include <vector>
#include "../util/api.hpp"
#include <contextualplanner/types/factcondition.hpp>
#include <contextualplanner/types/worldmodification.hpp>


namespace cp
{


/// Axiomatic thing that the bot can do
struct CONTEXTUALPLANNER_API Action
{
  /**
   * @brief Construct an Action.
   * @param pPreconditions Set of facts that should be present in the world to be able to do this action.
   * @param pEffect How the world will change when this action will be finished.
   * @param pPreferInContext Set of facts that will increase the priority of this action if they are present in the world.
   */
  Action(std::unique_ptr<FactCondition> pPrecondition,
         const WorldModification& pEffect,
         std::unique_ptr<FactCondition> pPreferInContext = {})
    : parameters(),
      precondition(pPrecondition ? std::move(pPrecondition) : std::unique_ptr<FactCondition>()),
      preferInContext(pPreferInContext ? std::move(pPreferInContext) : std::unique_ptr<FactCondition>()),
      effect(pEffect),
      highImportanceOfNotRepeatingIt(false)
  {
  }

  /// Move constructor of an action.
  Action(std::unique_ptr<FactCondition>&& pPrecondition,
         WorldModification&& pEffect,
         std::unique_ptr<FactCondition>&& pPreferInContext)
    : parameters(),
      precondition(pPrecondition ? std::move(pPrecondition) : std::unique_ptr<FactCondition>()),
      preferInContext(pPreferInContext ? std::move(pPreferInContext) : std::unique_ptr<FactCondition>()),
      effect(std::move(pEffect)),
      highImportanceOfNotRepeatingIt(false)
  {
  }

  Action(const Action& pAction)
    : parameters(pAction.parameters),
      precondition(pAction.precondition ? pAction.precondition->clone() : std::unique_ptr<FactCondition>()),
      preferInContext(pAction.preferInContext ? pAction.preferInContext->clone() : std::unique_ptr<FactCondition>()),
      effect(pAction.effect),
      highImportanceOfNotRepeatingIt(pAction.highImportanceOfNotRepeatingIt)
  {
  }

  void operator=(const Action& pAction)
  {
    parameters = pAction.parameters;
    precondition = pAction.precondition ? pAction.precondition->clone() : std::unique_ptr<FactCondition>();
    preferInContext = pAction.preferInContext ? pAction.preferInContext->clone() : std::unique_ptr<FactCondition>();
    effect = pAction.effect;
    highImportanceOfNotRepeatingIt = pAction.highImportanceOfNotRepeatingIt;
  }

  /// Check if this object contains a fact.
  bool hasFact(const cp::Fact& pFact) const;

  /**
   * @brief Replace a fact by another inside this object.
   * @param pOldFact Fact to replace.
   * @param pNewFact New fact value to set.
   */
  void replaceFact(const cp::Fact& pOldFact,
                   const cp::Fact& pNewFact);

  std::string precondition_str() const { return precondition ? precondition->toStr() : ""; }
  std::string preferInContext_str() const { return preferInContext ? preferInContext->toStr() : ""; }

  /// Parameter names of this action.
  std::vector<std::string> parameters;
  /// Set of facts that should be present in the world to be able to do this action.
  std::unique_ptr<FactCondition> precondition;
  /// Set of facts that will increase the priority of this action if they are present in the world.
  std::unique_ptr<FactCondition> preferInContext;
  /// How the world will change when this action will be finished.
  WorldModification effect;
  /// If it is important to not repeat this action.
  bool highImportanceOfNotRepeatingIt = false;
};


} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_ACTION_HPP
