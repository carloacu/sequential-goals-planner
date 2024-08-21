#include "test_facttoconditions.hpp"
#include <assert.h>
#include <contextualplanner/types/fact.hpp>
#include <contextualplanner/types/facttoconditions.hpp>
#include <contextualplanner/types/ontology.hpp>
#include <contextualplanner/util/alias.hpp>

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

}


void test_factToConditions()
{
  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity\n"
                                           "my_type my_type2 - entity\n"
                                           "sub_my_type2 - my_type2");
  ontology.constants = cp::SetOfEntities::fromStr("toto toto2 - my_type\n"
                                                  "titi titi_const - my_type2", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("pred_name(?e - entity)\n"
                                                     "pred_name2(?e - entity)\n"
                                                     "pred_name3(?p1 - my_type, ?p2 - my_type2)\n"
                                                     "pred_name4(?p1 - my_type, ?p2 - my_type2)\n"
                                                     "pred_name5(?p1 - my_type) - my_type2\n"
                                                     "pred_name6(?e - entity)\n",
                                                     ontology.types);

  auto entities = cp::SetOfEntities::fromStr("toto3 - my_type\n"
                                             "titi2 - my_type2\n"
                                             "titi3 - sub_my_type2", ontology.types);

  FactToConditions factToActions;

  auto fact1 = cp::Fact::fromStr("pred_name(toto)", ontology, entities, {});
  fact1.ensureAllFactAccessorCacheAreSet();
  factToActions.add(fact1, "action1");

  {
    assert_eq<std::string>("[action1]", factToActions.find(fact1).toStr());
  }

  {
    std::vector<cp::Parameter> parameters(1, cp::Parameter::fromStr("?p - my_type", ontology.types));
    auto factWithParam = cp::Fact::fromStr("pred_name(?p)", ontology, entities, parameters);
    factWithParam.ensureAllFactAccessorCacheAreSet();
    assert_eq<std::string>("[action1]", factToActions.find(factWithParam).toStr());
  }

  auto fact2 = cp::Fact::fromStr("pred_name(toto2)", ontology, entities, {});
  fact2.ensureAllFactAccessorCacheAreSet();
  factToActions.add(fact2, "action2");

  auto fact2b = cp::Fact::fromStr("pred_name6(toto)", ontology, entities, {});
  fact2b.ensureAllFactAccessorCacheAreSet();
  factToActions.add(fact2b, "action2");

  {
    auto factWithParam = cp::Fact::fromStr("pred_name6(toto)", ontology, entities, {});
    factWithParam.ensureAllFactAccessorCacheAreSet();
    assert_eq<std::string>("[action2]", factToActions.find(factWithParam).toStr());
  }

  {
    std::vector<cp::Parameter> parameters(1, cp::Parameter::fromStr("?p - my_type", ontology.types));
    auto factWithParam = cp::Fact::fromStr("pred_name(?p)", ontology, entities, parameters);
    factWithParam.ensureAllFactAccessorCacheAreSet();
    assert_eq<std::string>("[action1, action2]", factToActions.find(factWithParam).toStr());
  }

  {
    std::vector<cp::Parameter> parameters(1, cp::Parameter::fromStr("?p - my_type", ontology.types));
    auto factWithParam = cp::Fact::fromStr("pred_name2(?p)", ontology, entities, parameters);
    factWithParam.ensureAllFactAccessorCacheAreSet();
    assert_eq<std::string>("[]", factToActions.find(factWithParam).toStr());
  }

  {
    std::vector<cp::Parameter> parameters(1, cp::Parameter::fromStr("?p2 - my_type2", ontology.types));
    auto factWithParam = cp::Fact::fromStr("pred_name3(toto, ?p2)", ontology, entities, parameters);
    factWithParam.ensureAllFactAccessorCacheAreSet();
    assert_eq<std::string>("[]", factToActions.find(factWithParam).toStr());
  }

  auto fact3 = cp::Fact::fromStr("pred_name3(toto, titi)", ontology, entities, {});
  fact3.ensureAllFactAccessorCacheAreSet();
  factToActions.add(fact3, "action3");

  {
    std::vector<cp::Parameter> parameters(1, cp::Parameter::fromStr("?p2 - my_type2", ontology.types));
    auto factWithParam = cp::Fact::fromStr("pred_name3(toto, ?p2)", ontology, entities, parameters);
    factWithParam.ensureAllFactAccessorCacheAreSet();
    assert_eq<std::string>("[action3]", factToActions.find(factWithParam).toStr());
  }

  std::vector<cp::Parameter> fact4Parameters(1, cp::Parameter::fromStr("?p - my_type2", ontology.types));
  auto fact4 = cp::Fact::fromStr("pred_name3(toto, ?p)", ontology, entities, fact4Parameters);
  fact4.ensureAllFactAccessorCacheAreSet();
  factToActions.add(fact4, "action4");

  {
    std::vector<cp::Parameter> parameters(1, cp::Parameter::fromStr("?p2 - my_type2", ontology.types));
    auto factWithParam = cp::Fact::fromStr("pred_name3(toto, ?p2)", ontology, entities, parameters);
    factWithParam.ensureAllFactAccessorCacheAreSet();
    assert_eq<std::string>("[action3, action4]", factToActions.find(factWithParam).toStr());
  }

  {
    auto factWithParam = cp::Fact::fromStr("pred_name3(toto, titi)", ontology, entities, {});
    factWithParam.ensureAllFactAccessorCacheAreSet();
    assert_eq<std::string>("[action3, action4]", factToActions.find(factWithParam).toStr());
  }

  std::vector<cp::Parameter> fact4bParameters(1, cp::Parameter::fromStr("?p - my_type2", ontology.types));
  auto fact4b = cp::Fact::fromStr("pred_name3(toto, ?p)", ontology, entities, fact4bParameters);
  fact4b.ensureAllFactAccessorCacheAreSet();
  factToActions.add(fact4b, "action4b");

  {
    std::vector<cp::Parameter> parameters(1, cp::Parameter::fromStr("?p1 - my_type", ontology.types));
    auto factWithParam = cp::Fact::fromStr("pred_name3(?p1, titi)", ontology, entities, parameters);
    factWithParam.ensureAllFactAccessorCacheAreSet();
    assert_eq<std::string>("[action3, action4, action4b]", factToActions.find(factWithParam).toStr());
  }

  {
    std::vector<cp::Parameter> parameters(1, cp::Parameter::fromStr("?p1 - my_type", ontology.types));
    auto factWithParam = cp::Fact::fromStr("pred_name3(?p1, titi2)", ontology, entities, parameters);
    factWithParam.ensureAllFactAccessorCacheAreSet();
    assert_eq<std::string>("[action4, action4b]", factToActions.find(factWithParam).toStr());
  }

  {
    auto factWithParam = cp::Fact::fromStr("pred_name3(toto3, titi2)", ontology, entities, {});
    factWithParam.ensureAllFactAccessorCacheAreSet();
    assert_eq<std::string>("[]", factToActions.find(factWithParam).toStr());
  }

  {
    std::vector<cp::Parameter> parameters(1, cp::Parameter::fromStr("?p1 - my_type", ontology.types));
    auto factWithParam = cp::Fact::fromStr("pred_name3(?p1, titi3)", ontology, entities, parameters);
    factWithParam.ensureAllFactAccessorCacheAreSet();
    assert_eq<std::string>("[action4, action4b]", factToActions.find(factWithParam).toStr());
  }

  auto fact5 = cp::Fact::fromStr("pred_name3(toto, titi_const)", ontology, entities, {});
  fact5.ensureAllFactAccessorCacheAreSet();
  factToActions.add(fact5, "action5");

  auto fact6 = cp::Fact::fromStr("pred_name3(toto2, titi)", ontology, entities, {});
  fact6.ensureAllFactAccessorCacheAreSet();
  factToActions.add(fact6, "action6");

  {
    auto factWithParam = cp::Fact::fromStr("pred_name3(toto, titi)", ontology, entities, {});
    factWithParam.ensureAllFactAccessorCacheAreSet();
    assert_eq<std::string>("[action3, action4, action4b]", factToActions.find(factWithParam).toStr());
  }

  // tests with pred_name4

  auto fact7 = cp::Fact::fromStr("pred_name4(toto, titi_const)", ontology, entities, {});
  fact7.ensureAllFactAccessorCacheAreSet();
  factToActions.add(fact7, "action7");

  std::vector<cp::Parameter> fact8Parameters(1, cp::Parameter::fromStr("?p2 - my_type2", ontology.types));
  auto fact8 = cp::Fact::fromStr("pred_name4(toto, ?p2)", ontology, entities, fact8Parameters);
  fact8.ensureAllFactAccessorCacheAreSet();
  factToActions.add(fact8, "action8");

  std::vector<cp::Parameter> fact9Parameters(1, cp::Parameter::fromStr("?p2 - my_type2", ontology.types));
  auto fact9 = cp::Fact::fromStr("pred_name4(toto2, ?p2)", ontology, entities, fact9Parameters);
  fact9.ensureAllFactAccessorCacheAreSet();
  factToActions.add(fact9, "action9");

  {
    auto factWithParam = cp::Fact::fromStr("pred_name4(toto, titi)", ontology, entities, {});
    factWithParam.ensureAllFactAccessorCacheAreSet();
    assert_eq<std::string>("[action8]", factToActions.find(factWithParam).toStr());
  }


  // tests with pred_name5

  auto fact10 = cp::Fact::fromStr("pred_name5(toto2)=titi", ontology, entities, {});
  fact10.ensureAllFactAccessorCacheAreSet();
  factToActions.add(fact10, "action10");

  std::vector<cp::Parameter> fact11Parameters(1, cp::Parameter::fromStr("?p1 - my_type", ontology.types));
  auto fact11 = cp::Fact::fromStr("pred_name5(?p1)=titi", ontology, entities, fact11Parameters);
  fact11.ensureAllFactAccessorCacheAreSet();
  factToActions.add(fact11, "action11");

  std::vector<cp::Parameter> fact12Parameters(1, cp::Parameter::fromStr("?p1 - my_type", ontology.types));
  auto fact12 = cp::Fact::fromStr("pred_name5(?p1)=titi_const", ontology, entities, fact12Parameters);
  fact12.ensureAllFactAccessorCacheAreSet();
  factToActions.add(fact12, "action12");

  {
    auto factWithParam = cp::Fact::fromStr("pred_name5(toto)=titi", ontology, entities, {});
    factWithParam.ensureAllFactAccessorCacheAreSet();
    assert_eq<std::string>("[action11]", factToActions.find(factWithParam).toStr());
    assert_eq<std::string>("[action11, action12]", factToActions.find(factWithParam, true).toStr());
  }

  auto fact13 = cp::Fact::fromStr("pred_name5(toto2)!=titi", ontology, entities, {});
  fact13.ensureAllFactAccessorCacheAreSet();
  factToActions.add(fact13, "action13");

  {
    auto factWithParam = cp::Fact::fromStr("pred_name5(toto)=titi_const", ontology, entities, {});
    factWithParam.ensureAllFactAccessorCacheAreSet();
    assert_eq<std::string>("[action12, action13]", factToActions.find(factWithParam).toStr());
  }

  /*
  {
    auto factWithParam = cp::Fact::fromStr("pred_name5(toto)!=titi_const", ontology, entities, {});
    factWithParam.ensureAllFactAccessorCacheAreSet();
    assert_eq<std::string>("[action11]", factToActions.find(factWithParam).toStr());
    assert_eq<std::string>("[action11, action12]", factToActions.find(factWithParam, true).toStr());
  }
  */
}
