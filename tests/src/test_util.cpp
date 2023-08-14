#include "test_util.hpp"
#include <iostream>
#include <assert.h>
#include <contextualplanner/util/util.hpp>

using namespace cp;

namespace
{
template <typename TYPE>
void assert_eq(const TYPE& pExpected,
               const TYPE& pValue)
{
  if (pExpected != pValue)
    assert(false);
}


std::string _toStr(const std::list<std::map<std::string, std::string>>& pParams)
{
  std::string res;
  for (auto& currParams : pParams)
  {
    res += "(";
    bool firstIeration = true;
    for (const auto& currParam : currParams)
    {
      if (firstIeration)
        firstIeration = false;
      else
        res += ", ";
      res += currParam.first + " -> " + currParam.second;
    }
    res += ")";
  }
  return res;
}

std::string _unfoldMapWithSet(const std::map<std::string, std::set<std::string>>& pInMap)
{
  std::list<std::map<std::string, std::string>> res;
  unfoldMapWithSet(res, pInMap);
  return _toStr(res);
}

}

void test_unfoldMapWithSet()
{
  assert_eq<std::string>("", _unfoldMapWithSet({}));
  assert_eq<std::string>("(a -> b)", _unfoldMapWithSet({{"a", {"b"}}}));
  assert_eq<std::string>("(a -> b)(a -> c)", _unfoldMapWithSet({{"a", {"b", "c"}}}));

  assert_eq<std::string>("(a -> b, d -> e)",
                         _unfoldMapWithSet({{"a", {"b"}}, {"d", {"e"}}}));
  assert_eq<std::string>("(a -> b, d -> e)(a -> c, d -> e)",
                         _unfoldMapWithSet({{"a", {"b", "c"}}, {"d", {"e"}}}));
  assert_eq<std::string>("(a -> b, d -> e)(a -> b, d -> f)(a -> c, d -> e)(a -> c, d -> f)",
                         _unfoldMapWithSet({{"a", {"b", "c"}}, {"d", {"e", "f"}}}));
}
