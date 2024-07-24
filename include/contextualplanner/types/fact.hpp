#ifndef INCLUDE_CONTEXTUALPLANNER_FACT_HPP
#define INCLUDE_CONTEXTUALPLANNER_FACT_HPP

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <set>
#include "../util/api.hpp"
#include <contextualplanner/types/entity.hpp>
#include <contextualplanner/types/predicate.hpp>

namespace cp
{
struct FactOptional;
struct Ontology;
struct SetOfEntities;

/// Axiomatic knowledge that can be contained in a world.
struct CONTEXTUALPLANNER_API Fact
{
  /**
   * @brief Construct a fact.
   * @param[in] pStr Fact to construct serialized in a string.
   * @param[in] pSeparatorPtr Character to indicate the end of the fact in pStr. Nullptr can be set if there is only one fact in the string.
   * @param[out] pIsFactNegatedPtr Is the fact constructed negated or not.
   * @param[in] pBeginPos Begin position in pStr.
   * @param[out] pResPos End postion of the fact in pStr after the parsing.
   */
  Fact(const std::string& pStr,
       const Ontology& pOntology,
       const SetOfEntities& pEntities,
       const char* pSeparatorPtr = nullptr,
       bool* pIsFactNegatedPtr = nullptr,
       std::size_t pBeginPos = 0,
       std::size_t* pResPos = nullptr);

  Fact(const std::string& pName,
       const std::vector<std::string>& pArgumentStrs,
       const std::string& pFluentStr,
       const Ontology& pOntology,
       const SetOfEntities& pEntities);

  /// Destruct the fact.
  ~Fact();

  Fact(const Fact& pOther);
  Fact(Fact&& pOther) noexcept;
  Fact& operator=(const Fact& pOther);
  Fact& operator=(Fact&& pOther) noexcept;

  /// Specify an order beween facts. It alows to use this type as key of map containers.
  bool operator<(const Fact& pOther) const;

  /// Check equality with another fact.
  bool operator==(const Fact& pOther) const;
  /// Check not equality with another fact.
  bool operator!=(const Fact& pOther) const { return !operator==(pOther); }

  /**
   * @brief Check equality with another fact without considering the values.
   * @param[in] pFact Other fact to compare.
   * @param[in] pOtherFactParametersToConsiderAsAnyValuePtr Other fact arguments to consider as "any value".
   * @param[in] pOtherFactParametersToConsiderAsAnyValuePtr2 Another set of other fact rguments to consider as "any value".
   * @return True if the equality check succeeded.
   */
  bool areEqualWithoutFluentConsideration(const Fact& pFact,
                                          const std::map<std::string, std::set<Entity>>* pOtherFactParametersToConsiderAsAnyValuePtr = nullptr,
                                          const std::map<std::string, std::set<Entity>>* pOtherFactParametersToConsiderAsAnyValuePtr2 = nullptr) const;

  /// Check equality with another fact without considering an argument.
  bool areEqualWithoutAnArgConsideration(const Fact& pFact,
                                         const std::string& pArgToIgnore) const;

  /**
   * @brief Is equal to another Fact or if any of the 2 Facts have an "any value" that can match.
   * @param[in] pOther Other fact to compare.
   * @param[in] pOtherFactArgumentsToConsiderAsAnyValuePtr Arguments to consider as "any value".
   * @param[in] pOtherFactParametersToConsiderAsAnyValuePtr2 Another set of arguments to consider as "any value".
   * @param[in] pThisArgumentsToConsiderAsAnyValuePtr Arguments of the this fact to consider as "any value".
   * @return True if the 2 facts match, false otherwise.
   */
  bool areEqualExceptAnyValues(const Fact& pOther,
                               const std::map<std::string, std::set<Entity>>* pOtherFactParametersToConsiderAsAnyValuePtr = nullptr,
                               const std::map<std::string, std::set<Entity>>* pOtherFactParametersToConsiderAsAnyValuePtr2 = nullptr,
                               const std::vector<std::string>* pThisFactParametersToConsiderAsAnyValuePtr = nullptr) const;

  /**
   * @brief Is equal to another Fact or if any of the 2 Facts have an "any value" that can match and without looking at the fluents.
   * @param[in] pOther Other fact to compare.
   * @param[in] pOtherFactArgumentsToConsiderAsAnyValuePtr Arguments to consider as "any value".
   * @param[in] pOtherFactParametersToConsiderAsAnyValuePtr2 Another set of arguments to consider as "any value".
   * @param[in] pThisArgumentsToConsiderAsAnyValuePtr Arguments of the this fact to consider as "any value".
   * @return True if the 2 facts match, false otherwise.
   */
  bool areEqualExceptAnyValuesAndFluent(const Fact& pOther,
                                        const std::map<std::string, std::set<Entity>>* pOtherFactParametersToConsiderAsAnyValuePtr = nullptr,
                                        const std::map<std::string, std::set<Entity>>* pOtherFactParametersToConsiderAsAnyValuePtr2 = nullptr,
                                        const std::vector<std::string>* pThisFactParametersToConsiderAsAnyValuePtr = nullptr) const;

  /**
   * @brief Is it a punctual fact.<br/>
   * A punctual fact is a fact that is considered punctually but never stored in the world.
   * @return True if the fact is punctual.
   */
  bool isPunctual() const;

  /**
   * @brief hasArgumentOrValue Does this fact contains a specific string in his arguments or in his value.
   * @param pArgumentOrValue[in] String that can match in the arguments or in the value of this fact.
   * @return True if the string matches in the arguments or in the value of this fact, false otherwise.
   */
  bool hasArgumentOrValue(
      const std::string& pArgumentOrValue) const;

  /**
   * @brief Extract an argument from another instance of this fact.<br/>
   * Another instance of this fact means that the 2 facts have the same name, the same number of arguments and the same polarity (= negationed or not).
   * @param pArgument[in] Argument of this fact.
   * @param pExampleFact[in] Example Fact.
   * @return Argument of the other fact corresponding to the pArgument of this fact.
   */
  std::optional<Entity> tryToExtractArgumentFromExample(
      const std::string& pArgument,
      const Fact& pExampleFact) const;

  /**
   * @brief Extract an argument from another instance of this fact.<br/>
   * Another instance of this fact means that the 2 facts have the same name, the same number of arguments and the same polarity (= negationed or not).<br/>
   * This function ignores the fluents.
   * @param pArgument[in] Argument of this fact.
   * @param pExampleFact[in] Example Fact.
   * @return Argument of the other fact corresponding to the pArgument of this fact.
   */
  std::optional<Entity> tryToExtractArgumentFromExampleWithoutFluentConsideration(
      const std::string& pArgument,
      const Fact& pExampleFact) const;


  /**
   * @brief isPatternOf Does this fact is a generic form of the fact example according to the possible arguments.
   * @param pPossibleArguments[in] Possible arguments limitation.
   * @param pFactExample[in] The fact in example.
   * @return True if this fact is a generic form of the fact example according to the possible arguments.
   */
  bool isPatternOf(const std::map<std::string, std::set<Entity>>& pPossibleArguments,
                   const Fact& pFactExample) const;

  /**
   * @brief Replace some arguments by other ones.
   * @param pCurrentArgumentsToNewArgument[in] Map of current arguments to new argument to set.
   */
  void replaceArguments(const std::map<std::string, Entity> &pCurrentArgumentsToNewArgument);

  /**
   * @brief Replace some arguments by other ones.
   * @param pCurrentArgumentsToNewArgument[in] Map of current arguments to new possible arguments to set.<br/>
   * Only the first new possible argument to set will be considered.
   */
  void replaceArguments(const std::map<std::string, std::set<Entity>>& pCurrentArgumentsToNewArgument);


  /// Serialize this fact to a string.
  std::string toStr() const;

  /**
   * @brief Construct a fact from a string.
   * @param pStr Input string.
   * @param pIsFactNegatedPtr Is the fact constructed negated or not.
   * @return Fact constructed.
   */
  static Fact fromStr(const std::string& pStr,
                      const Ontology& pOntology,
                      const SetOfEntities& pEntities,
                      bool* pIsFactNegatedPtr = nullptr);

  /**
   * @brief Deserialize a part of a string to this fact.
   * @param pStr[in] String containing the part to deserialize.
   * @param pSeparatorPtr[in] Character to indicate the end of the fact in pStr. Nullptr can be set if there is only one fact in the string.
   * @param pBeginPos[in] Begin position in pStr.
   * @param pIsFactNegatedPtr[out] Is the fact constructed negated or not.
   * @return End index of the deserialization.
   */
  std::size_t fillFactFromStr(const std::string& pStr,
                              const Ontology& pOntology,
                              const SetOfEntities& pEntities,
                              const char* pSeparatorPtr,
                              std::size_t pBeginPos,
                              bool* pIsFactNegatedPtr);

  /**
   * @brief Set "any value" to all of the specified arguments.
   * @param pArgumentsToReplace[in] Arguments to replace by "any value".
   * @return True if at least one "any value" has been set, false otherwise.
   */
  bool replaceSomeArgumentsByAny(const std::vector<std::string>& pArgumentsToReplace);

  /**
   * @brief Does the fact matches any of the other facts.
   * @param[in] pOtherFacts Set of other facts.
   * @param[in] pParametersAreForTheFact If true, get the parameters from the fact else get the parameters from the set of other facts.
   * @param[out] pNewParametersPtr New parameter possibilities corresponding of the found match.
   * @param[in] pParametersPtr Already known parameters.
   * @param[in, out] pParametersToModifyInPlacePtr Parameters to modify in place.
   * @param[in] pTriedToModifyParametersPtr True if pNewParametersPtr is nullptr and this function wanted to add new parameters.
   * @return True if the fact matches any of the other facts.
   */
  bool isInOtherFacts(const std::set<Fact>& pOtherFacts,
                      bool pParametersAreForTheFact,
                      std::map<std::string, std::set<Entity>>* pNewParametersPtr,
                      const std::map<std::string, std::set<Entity>>* pParametersPtr,
                      std::map<std::string, std::set<Entity>>* pParametersToModifyInPlacePtr = nullptr,
                      bool* pTriedToModifyParametersPtr = nullptr) const;

  /**
   * @brief Does the fact matches any of the other facts.
   * @param[in] pOtherFacts Map of fact names to other facts.
   * @param[in] pParametersAreForTheFact If true, get the parameters from the fact else get the parameters from the set of other facts.
   * @param[out] pNewParametersPtr New parameter possibilities corresponding of the found match.
   * @param[in] pParametersPtr Already known parameters.
   * @param[in, out] pParametersToModifyInPlacePtr Parameters to modify in place.
   * @param[in] pTriedToModifyParametersPtr True if pNewParametersPtr is nullptr and this function wanted to add new parameters.
   * @return True if the fact matches any of the other facts.
   */
  bool isInOtherFactsMap(const std::map<std::string, std::set<Fact>>& pOtherFacts,
                         bool pParametersAreForTheFact,
                         std::map<std::string, std::set<Entity>>* pNewParametersPtr,
                         const std::map<std::string, std::set<Entity>>* pParametersPtr,
                         std::map<std::string, std::set<Entity>>* pParametersToModifyInPlacePtr = nullptr,
                         bool* pTriedToModifyParametersPtr = nullptr) const;

  /**
   * @brief Does the fact matches the other fact.
   * @param[in] pOtherFact The other facts.
   * @param[in] pParametersAreForTheFact If true, get the parameters from the fact else get the parameters from the other fact.
   * @param[out] pNewParametersPtr New parameter possibilities corresponding of the found match.
   * @param[in] pParametersPtr Already known parameters.
   * @param[in, out] pParametersToModifyInPlacePtr Parameters to modify in place.
   * @param[in] pTriedToModifyParametersPtr True if pNewParametersPtr is nullptr and this function wanted to add new parameters.
   * @param[in] pIgnoreFluents If we should ignore the fluents.
   * @return True if the fact matches the other fact.
   */
  bool isInOtherFact(const Fact& pOtherFact,
                     bool pParametersAreForTheFact,
                     std::map<std::string, std::set<Entity>>* pNewParametersPtr,
                     const std::map<std::string, std::set<Entity>>* pParametersPtr,
                     std::map<std::string, std::set<Entity>>* pParametersToModifyInPlacePtr,
                     bool* pTriedToModifyParametersPtr = nullptr,
                     bool pIgnoreFluents = false) const;

  /**
   * @brief Replace, in the arguments of this fact, a fact by another fact.
   * @param pCurrent Fact to search in the arguments.
   * @param pNew New fact to set in place of pCurrent.
   */
  void replaceArgument(const std::string& pCurrent,
                       const std::string& pNew);

  /// Name of the fact.
  std::string name;
  /// Arguments of the fact.
  std::vector<Entity> arguments;
  /// Fluent of the fact.
  std::optional<Entity> fluent;
  /// Is the value of the fact negated.
  bool isValueNegated;
  Predicate predicate;

  /// Constant defining the "any value" special value.
  const static Entity anyValue;
  /// Constant defining the "undefined" special value.
  const static Entity undefinedValue;
  /// Prefix to detect a punctual fact. (= fact that is considered punctually but not stored in the world)
  static std::string punctualPrefix;

private:

  void _addArgument(const std::string& pArgumentName,
                    const Ontology& pOntology,
                    const SetOfEntities& pEntities);
  void _setFluent(const std::string& pFluentStr,
                  const Ontology& pOntology,
                  const SetOfEntities& pEntities);
  void _finalizeInisilizationAndValidityChecks(const Ontology& pOntology,
                                               const SetOfEntities& pEntities);
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_FACT_HPP
