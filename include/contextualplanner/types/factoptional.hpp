#ifndef INCLUDE_CONTEXTUALPLANNER_FACTOPTIONAL_HPP
#define INCLUDE_CONTEXTUALPLANNER_FACTOPTIONAL_HPP

#include <functional>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "../util/api.hpp"
#include "fact.hpp"

namespace cp
{

/// Fact that can be negated.
struct CONTEXTUALPLANNER_API FactOptional
{
  /**
   * @brief Construct an optional fact.
   * @param pFact Fact contained.
   * @param pIsFactNegated If the fact is negated.
   */
  FactOptional(const Fact& pFact,
               bool pIsFactNegated = false);
  /**
   * @brief Construct an optional fact from another one.
   * @param pOther Other optional fact.
   */
  FactOptional(const FactOptional& pOther,
               const std::map<std::string, std::string>* pParametersPtr = nullptr);

  /**
   * @brief Construct an optional fact from a part of a string.
   * @param[in] pStr Input string to parse.
   * @param[in] pSeparatorPtr Character to indicate the end of the fact in the input string, nullptr can be set if there is only one.
   * @param[in] pBeginPos Begin position in the input string.
   * @param[out] pResPos Postion in the input string after the parse.
   */
  FactOptional(const std::string& pStr,
               const char* pSeparatorPtr = nullptr,
               std::size_t pBeginPos = 0,
               std::size_t* pResPos = nullptr);

  /// Set content from another optional fact.
  void operator=(const FactOptional& pOther);

  /// Check equality with another optional fact.
  bool operator==(const FactOptional& pOther) const;
  /// Check not equality with another optional fact.
  bool operator!=(const FactOptional& pOther) const { return !operator==(pOther); }

  /// Serialize this optional fact to a string.
  std::string toStr(const std::function<std::string(const Fact&)>* pFactWriterPtr = nullptr) const;

  /// Is the fact negated.
  bool isFactNegated;
  /// Fact contained in this goal.
  Fact fact;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_FACTOPTIONAL_HPP
