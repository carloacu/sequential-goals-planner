#ifndef INCLUDE_CONTEXTUALPLANNER_FACT_HPP
#define INCLUDE_CONTEXTUALPLANNER_FACT_HPP

#include <string>
#include <vector>
#include <map>
#include <set>
#include "../util/api.hpp"


namespace cp
{
struct FactOptional;


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
       const char* pSeparatorPtr = nullptr,
       bool* pIsFactNegatedPtr = nullptr,
       std::size_t pBeginPos = 0,
       std::size_t* pResPos = nullptr);

  /// Destruct the fact.
  ~Fact();

  /// Specify an order beween facts. It alows to use this type as key of map containers.
  bool operator<(const Fact& pOther) const;

  /// Check equality with another fact.
  bool operator==(const Fact& pOther) const;
  /// Check not equality with another fact.
  bool operator!=(const Fact& pOther) const { return !operator==(pOther); }

  /// Check equality with another fact without considering the values.
  bool areEqualWithoutValueConsideration(const Fact& pFact) const;

  /**
   * @brief Is equal to another Fact or if any of the 2 Facts have an "any value" that can match.
   * @param pOther[in] Other fact to compare.
   * @param pOtherFactArgumentsToConsiderAsAnyValuePtr[in] Arguments of the other fact to consider as "any value".
   * @param pThisArgumentsToConsiderAsAnyValuePtr[in] Arguments of the this fact to consider as "any value".
   * @return True if the 2 facts match, false otherwise.
   */
  bool areEqualExceptAnyValues(const Fact& pOther,
                               const std::map<std::string, std::set<std::string>>* pOtherFactParametersToConsiderAsAnyValuePtr = nullptr,
                               const std::vector<std::string>* pThisFactParametersToConsiderAsAnyValuePtr = nullptr) const;

  /**
   * @brief Is it a punctual fact.<br/>
   * A punctual fact is a fact that is considered punctually but never stored in the world.
   * @return True if the fact is punctual.
   */
  bool isPunctual() const;

  /**
   * @brief Is it an unreachable fact.<br/>
   * An unreachable fact is a fact that is neither considered punctually nor stored in the world.<br/>
   * But an ureachable fact is considered for planning deduction.
   * @return True if the fact is unreachable.
   */
  bool isUnreachable() const;

  /**
   * @brief Extract an argument from another instance of this fact.<br/>
   * Another instance of this fact means that the 2 facts have the same name, the same number of arguments and the same polarity (= negationed or not).
   * @param pArgument[in] Argument of this fact.
   * @param pOther[in] Other Fact.
   * @return Argument of the other fact corresponding to the pArgument of this fact.
   */
  std::string tryToExtractArgumentFromExample(
      const std::string& pArgument,
      const Fact& pOther) const;

  /**
   * @brief Replace some arguments by other ones.
   * @param pCurrentArgumentsToNewArgument[in] Map of current arguments to new argument to set.
   */
  void replaceArguments(
      const std::map<std::string, std::string>& pCurrentArgumentsToNewArgument);

  /**
   * @brief Replace some arguments by other ones.
   * @param pCurrentArgumentsToNewArgument[in] Map of current arguments to new possible arguments to set.<br/>
   * Only the first new possible argument to set will be considered.
   */
  void replaceArguments(
      const std::map<std::string, std::set<std::string>>& pCurrentArgumentsToNewArgument);


  /// Serialize this fact to a string.
  std::string toStr() const;

  /**
   * @brief Construct a fact from a string.
   * @param pStr Input string.
   * @param pIsFactNegatedPtr Is the fact constructed negated or not.
   * @return Fact constructed.
   */
  static Fact fromStr(const std::string& pStr,
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
   * @param[in] pTriedToModifyParametersPtr True if pNewParametersPtr is nullptr and this function wanted to add new parameters.
   * @return True if the fact matches any of the other facts.
   */
  bool isInOtherFacts(const std::set<Fact>& pOtherFacts,
                      bool pParametersAreForTheFact,
                      std::map<std::string, std::set<std::string>>* pNewParametersPtr,
                      const std::map<std::string, std::set<std::string>>* pParametersPtr,
                      bool* pTriedToModifyParametersPtr = nullptr) const;

  /**
   * @brief Does the fact matches the other fact.
   * @param[in] pOtherFact The other facts.
   * @param[in] pParametersAreForTheFact If true, get the parameters from the fact else get the parameters from the other fact.
   * @param[out] pNewParametersPtr New parameter possibilities corresponding of the found match.
   * @param[in] pParametersPtr Already known parameters.
   * @param[in] pTriedToModifyParametersPtr True if pNewParametersPtr is nullptr and this function wanted to add new parameters.
   * @return True if the fact matches the other fact.
   */
  bool isInOtherFact(const Fact& pOtherFact,
                     bool pParametersAreForTheFact,
                     std::map<std::string, std::set<std::string>>* pNewParametersPtr,
                     const std::map<std::string, std::set<std::string>>* pParametersPtr,
                     bool* pTriedToModifyParametersPtr = nullptr) const;

  /**
   * @brief Replace, in the arguments of this fact, a fact by another fact.
   * @param pCurrentFact Fact to search in the arguments.
   * @param pNewFact New fact to set in place of pCurrentFact.
   */
  void replaceFactInArguments(const Fact& pCurrentFact,
                              const Fact& pNewFact);

  /// Name of the fact.
  std::string name;
  /// Arguments of the fact.
  std::vector<FactOptional> arguments;
  /// Value of the fact.
  std::string value;
  /// Is the value of the fact negated.
  bool isValueNegated;

  /// Constant defining the "any value" special value.
  const static std::string anyValue;
  /// Constant defining the "any value" special fact.
  const static FactOptional anyValueFact;
  /// Prefix to detect a punctual fact. (= fact that is considered punctually but not stored in the world)
  static std::string punctualPrefix;
  /// Prefix to detect a unreachable fact. (= fact that is neither considered punctually nor stored in the world but that can be used for deductions)
  static std::string unreachablePrefix;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_FACT_HPP
