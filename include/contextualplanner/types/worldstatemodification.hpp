#ifndef INCLUDE_CONTEXTUALPLANNER_WORLDSTATEMODIFICATION_HPP
#define INCLUDE_CONTEXTUALPLANNER_WORLDSTATEMODIFICATION_HPP

#include <functional>
#include <memory>
#include "factoptional.hpp"
#include "../util/api.hpp"


namespace cp
{
struct WorldState;


/// Modification of a world state represented in a tree structure to apply to a world state.
struct CONTEXTUALPLANNER_API WorldStateModification
{
  /**
   * @brief Create a world state modification from a string.
   * @param[in] pStr String to convert.
   * @return New world state modification created.
   */
  static std::unique_ptr<WorldStateModification> fromStr(const std::string& pStr);

  /**
   * @brief Create a world state modification by concatenating two existing world state modifications.
   * @param[in] pWsModif1 First world state modification to concatenate.
   * @param[in] pWsModif2 Second world state modification to concatenate.
   * @return New world state modification created.
   */
  static std::unique_ptr<WorldStateModification> createByConcatenation(const WorldStateModification& pWsModif1,
                                                                       const WorldStateModification& pWsModif2);

  /**
   * @brief Convert the world state modification to a string.
   * @return World state modification converted to a string.
   */
  virtual std::string toStr() const = 0;


  /// Check if this object contains a fact or the negation of the fact.
  virtual bool hasFact(const cp::Fact& pFact) const = 0;

  /// Does this object only represents a set of facts.
  virtual bool isOnlyASetOfFacts() const = 0;

  /**
   * @brief Replace a fact or the negation of the fact by another fact.
   * @param[in] pOldFact Existing fact to replace.
   * @param[in] pNewFact New fact to set instead.
   */
  virtual void replaceFact(const cp::Fact& pOldFact,
                           const Fact& pNewFact) = 0;

  /**
   * @brief Iterate over all the optional facts with fact value resolution according to the world state.
   * @param[in] pFactCallback Callback called for each optional fact of this object.
   */
  virtual void forAll(const std::function<void (const FactOptional&)>& pFactCallback,
                      const WorldState& pWorldState) const = 0;

  /**
   * @brief Iterate over all the optional facts with fact value resolution according to the world state until the callback returns true.
   * @param[in] pFactCallback Callback called for each optional fact of this object.
   */
  virtual bool forAllUntilTrue(const std::function<bool (const FactOptional&)>& pFactCallback,
                               const WorldState& pWorldState) const = 0;

  /// Equality operators.
  virtual bool operator==(const WorldStateModification& pOther) const = 0;
  virtual bool operator!=(const WorldStateModification& pOther) const { return !operator==(pOther); }

  /**
   * @brief Convert this world state modification to a value.
   * @param[in] pWorldState World state use to extract value of the facts.
   * @return The world state modification converted to a string value.
   */
  virtual std::string getValue(const WorldState& pWorldState) const = 0;

  /**
   * @brief Create a copy of this object with arguments filling (or not if pParametersToArgumentPtr is nullptr).
   * @param pParametersToArgumentPtr Parameters to replace by their argument in the new object to create.
   * @return A copy of this object with arguments filling.
   */
  virtual std::unique_ptr<WorldStateModification> clone(const std::map<std::string, std::string>* pParametersToArgumentPtr) const = 0;

  /**
   * @brief Create a copy of this object with arguments filling (or not if pParametersToPossibleArgumentPtr is nullptr).
   * @param pParametersToPossibleArgumentPtr Parameters to replace by the first of their possible arguments in the new object to create.
   * @return A copy of this object with arguments filling.
   */
  virtual std::unique_ptr<WorldStateModification> cloneParamSet(const std::map<std::string, std::set<std::string>>& pParametersToPossibleArgumentPtr) const = 0;
};


} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_WORLDSTATEMODIFICATION_HPP
