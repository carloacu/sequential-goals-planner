#include <gtest/gtest.h>
#include <prioritizedgoalsplanner/types/entity.hpp>
#include <prioritizedgoalsplanner/types/parameter.hpp>
#include <prioritizedgoalsplanner/util/util.hpp>

using namespace cp;

namespace
{

cp::Parameter _parameter(const std::string& pStr) {
  return cp::Parameter(pStr, {});
}

cp::Entity _entity(const std::string& pStr) {
  return cp::Entity(pStr, {});
}

std::string _toStr(const std::list<std::map<Parameter, Entity>>& pParams)
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
      res += currParam.first.name + " -> " + currParam.second.toStr();
    }
    res += ")";
  }
  return res;
}

std::string _unfoldMapWithSet(const std::map<Parameter, std::set<cp::Entity>>& pInMap)
{
  std::list<std::map<Parameter, cp::Entity>> res;
  unfoldMapWithSet(res, pInMap);
  return _toStr(res);
}

}

void test_unfoldMapWithSet()
{
  EXPECT_EQ("", _unfoldMapWithSet({}));
  EXPECT_EQ("(a -> b)", _unfoldMapWithSet({{_parameter("a"), {_entity("b")}}}));
  EXPECT_EQ("(a -> b)(a -> c)", _unfoldMapWithSet({{_parameter("a"), {_entity("b"), _entity("c")}}}));

  EXPECT_EQ("(a -> b, d -> e)",
            _unfoldMapWithSet({{_parameter("a"), {_entity("b")}}, {_parameter("d"), {_entity("e")}}}));
  EXPECT_EQ("(a -> b, d -> e)(a -> c, d -> e)",
            _unfoldMapWithSet({{_parameter("a"), {_entity("b"), _entity("c")}}, {_parameter("d"), {_entity("e")}}}));
  EXPECT_EQ("(a -> b, d -> e)(a -> b, d -> f)(a -> c, d -> e)(a -> c, d -> f)",
            _unfoldMapWithSet({{_parameter("a"), {_entity("b"), _entity("c")}}, {_parameter("d"), {_entity("e"), _entity("f")}}}));
}

void test_autoIncrementOfVersion()
{
  std::set<std::string> ids;
  auto isIdOkForInsertion = [&ids](const std::string& pId)
  {
    return ids.count(pId) == 0;
  };
  auto incrementAddIdAndReturnValue = [&](const std::string& pId)
  {
    auto newId = cp::incrementLastNumberUntilAConditionIsSatisfied(pId, isIdOkForInsertion);
    ids.insert(newId);
    return newId;
  };

  EXPECT_EQ("", cp::incrementLastNumberUntilAConditionIsSatisfied("", isIdOkForInsertion));
  EXPECT_EQ("dede", incrementAddIdAndReturnValue("dede"));
  EXPECT_EQ("dede_2", incrementAddIdAndReturnValue("dede"));
  EXPECT_EQ("dede_3", incrementAddIdAndReturnValue("dede"));
  EXPECT_EQ("dede_4", incrementAddIdAndReturnValue("dede_2"));
  EXPECT_EQ("dede_5", incrementAddIdAndReturnValue("dede_4"));
  EXPECT_EQ("dede_6", incrementAddIdAndReturnValue("dede_6"));
  EXPECT_EQ("didi", incrementAddIdAndReturnValue("didi"));
}


TEST(Tool, test_util)
{
  test_unfoldMapWithSet();
  test_autoIncrementOfVersion();
}
