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


/// Axiomatic knowledge that can be contained in the world.
struct CONTEXTUALPLANNER_API Fact
{
  /**
   * @brief Construct a fact.
   * @param[in] pStr Input string to parse.
   * @param[in] pSeparatorPtr Character to indicate the end of the fact in the input string, nullptr can be set if there is only one.
   * @param[out] pIsFactNegatedPtr Is the fact constructed negated or not.
   * @param[in] pBeginPos Begin position in the input string.
   * @param[out] pResPos Postion in the input string after the parse.
   */
  Fact(const std::string& pStr = "",
       const char* pSeparatorPtr = nullptr,
       bool* pIsFactNegatedPtr = nullptr,
       std::size_t pBeginPos = 0,
       std::size_t* pResPos = nullptr);

  /// Destruct the fact.
  ~Fact();

  /// Specify an order beween facts. It alows to use this type as key of map containers.
  bool operator<(const Fact& pOther) const;

  /// Check equality with another Fact.
  bool operator==(const Fact& pOther) const;
  /// Check not equality with another ExpressionElement.
  bool operator!=(const Fact& pOther) const { return !operator==(pOther); }

  /**
   * @brief Is it a punctual fact.<br/>
   * A punctual fact is a fact that cannot be stored in the world state.
   * @return True if the fact is punctual.
   */
  bool isPunctual() const;

  /**
   * @brief Is equal to another Fact or if any of the 2 Fact has an "any value" that can match.
   * @param pOther Other fact to compare.
   * @return True if the 2 facts match, false otherwise.
   */
  bool areEqualExceptAnyValues(const Fact& pOther) const;

  /**
   * @brief Extract a value of another fact from a parameter value of this fact.
   * @param pParameterValue Parameter value of this fact.
   * @param pOther Other Fact.
   * @return Value of the other fact cooresponding to the parameter value of this fact.
   */
  std::string tryToExtractParameterValueFromExemple(
      const std::string& pParameterValue,
      const Fact& pOther) const;

  /**
   * @brief Fill the parameters.
   * @param pParameters Parameters to fill.
   */
  void fillParameters(
      const std::map<std::string, std::string>& pParameters);

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
   * @param pStr String containing the part to deserialize.
   * @param pSeparatorPtr Character to specify the end of the fact, you can put nullptr if there is only one fact in the input string.
   * @param pBeginPos Begin index of the string for the deserialization.
   * @param pIsFactNegatedPtr Is the fact constructed negated or not.
   * @return End index of the deserialization.
   */
  std::size_t fillFactFromStr(const std::string& pStr,
                              const char* pSeparatorPtr,
                              std::size_t pBeginPos,
                              bool* pIsFactNegatedPtr);

  /**
   * @brief Set "any value" to all of the specified parameters.
   * @param pParameters Parameters to set "any value".
   * @return True if at least one "any value" has been set, false otherwise.
   */
  bool replaceParametersByAny(const std::vector<std::string>& pParameters);

  /**
   * @brief Is the fact present in a set of facts.
   * @param[in] pFacts Set of facts.
   * @param[in] pParametersAreForTheFact If true, get the parameters from the fact else get the parameters from the set of facts.
   * @param[in] pParameters Parameters to get.
   * @return True if the fact is in the facts.
   */
  bool isInFacts(const std::set<Fact>& pFacts,
                 bool pParametersAreForTheFact,
                 std::map<std::string, std::string>* pParametersPtr = nullptr) const;

  /// Name of the fact.
  std::string name;
  /// Parameters of the fact.
  std::vector<FactOptional> parameters;
  /// Value of the fact.
  std::string value;

  /// Constant defining the "any value" special value.
  const static std::string anyValue;
  /// Constant defining the "any value" special fact.
  const static FactOptional anyValueFact;
  /// Prefix to detect a punctual fact.
  static std::string punctualPrefix;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_FACT_HPP
