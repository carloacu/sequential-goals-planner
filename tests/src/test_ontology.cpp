#include "test_arithmeticevaluator.hpp"
#include <assert.h>
#include <iostream>
#include <contextualplanner/types/action.hpp>
#include <contextualplanner/types/fact.hpp>
#include <contextualplanner/types/goalstack.hpp>
#include <contextualplanner/types/ontology.hpp>
#include <contextualplanner/types/setofinferences.hpp>
#include <contextualplanner/types/worldstate.hpp>

namespace
{
template <typename TYPE>
void assert_eq(const TYPE& pExpected,
               const TYPE& pValue)
{
  if (pExpected != pValue)
    assert(false);
}

template <typename TYPE>
void assert_true(const TYPE& pValue)
{
  if (!pValue)
    assert(false);
}

template <typename TYPE>
void assert_false(const TYPE& pValue)
{
  if (pValue)
    assert(false);
}

std::map<cp::Parameter, std::set<cp::Entity>> _toParameterMap(const std::vector<cp::Parameter>& pParameters)
{
  std::map<cp::Parameter, std::set<cp::Entity>> res;
  for (auto& currParam : pParameters)
    res[currParam];
  return res;
}

void _test_setOfTypes()
{
  cp::SetOfTypes setOfTypes;
  assert_eq<std::string>("", setOfTypes.toStr());
  setOfTypes.addType("object");
  setOfTypes.addType("voiture", "object");
  setOfTypes.addType("maison", "object");
  setOfTypes.addType("citroen", "voiture");
  setOfTypes.addType("ferrari", "voiture");
  setOfTypes.addType("peugeot", "voiture");
  setOfTypes.addType("c3", "citroen");
  setOfTypes.addType("location");

  assert_eq<std::string>("voiture maison - object\n"
                         "citroen ferrari peugeot - voiture\n"
                         "c3 - citroen\n"
                         "location",
                         setOfTypes.toStr());
}


void _test_setOfTypes_fromStr()
{
  std::string typesStr = "voiture maison - object\n"
                         "citroen ferrari peugeot - voiture\n"
                         "c3 - citroen\n"
                         "location";
  auto setOfTypes = cp::SetOfTypes::fromStr(typesStr + " ");
  assert_eq<std::string>(typesStr, setOfTypes.toStr());
  assert_eq<std::string>("voiture", setOfTypes.nameToType("citroen")->parent->name);
}


void _test_predicateToStr()
{
  auto setOfTypes = cp::SetOfTypes::fromStr("my_type my_type2 return_type");
  assert_eq<std::string>("pred_name(?v - my_type)", cp::Predicate("pred_name(?v - my_type)", setOfTypes).toStr());
  assert_eq<std::string>("pred_name(?v - my_type, ?o - my_type2)", cp::Predicate("pred_name(?v - my_type, ?o - my_type2)", setOfTypes).toStr());
  assert_eq<std::string>("pred_name(?v - my_type, ?o - my_type2) - return_type", cp::Predicate("pred_name(?v - my_type, ?o - my_type2) - return_type", setOfTypes).toStr());

  auto otherSetOfTypes = cp::SetOfTypes::fromStr("a b");
  try {
    cp::Predicate("pred_name(?v - my_type)", otherSetOfTypes);
    assert_true(false);
  } catch (const std::exception& e) {
    assert_eq<std::string>("\"my_type\" is not a valid type name", e.what());
  }
  try {
    cp::Predicate("pred_name(?v - a, ?o - b) - return_type", otherSetOfTypes);
    assert_true(false);
  } catch (const std::exception& e) {
    assert_eq<std::string>("\"return_type\" is not a valid type name", e.what());
  }
}


void _test_setOfPredicates_fromStr()
{
  auto setOfTypes = cp::SetOfTypes::fromStr("my_type my_type2 return_type");
  std::string predicatesStr = "pred_name(?v - my_type)\n"
                              "pred_name2(?v - my_type, ?o - my_type2)";
  auto setOfPredicates = cp::SetOfPredicates::fromStr(predicatesStr, setOfTypes);
  assert_eq<std::string>(predicatesStr, setOfPredicates.toStr());
}


void _test_setOfEntities_fromStr()
{
  auto setOfTypes = cp::SetOfTypes::fromStr("my_type my_type2 return_type");
  std::string entitiesStr = "toto - my_type\n"
                            "titi tutu - my_type2";
  auto setOfEntities = cp::SetOfEntities::fromStr(entitiesStr, setOfTypes);
  assert_eq<std::string>(entitiesStr, setOfEntities.toStr());
  assert_eq<std::string>("my_type2", setOfEntities.valueToEntity("titi")->type->name);
}

void _test_fact_initialization()
{
  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("my_type my_type2 return_type\n"
                                           "sub_my_type - my_type");
  ontology.constants = cp::SetOfEntities::fromStr("toto - my_type\n"
                                                  "sub_toto - sub_my_type\n"
                                                  "titi tutu - my_type2\n"
                                                  "res - return_type", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("pred_name(?v - my_type)\n"
                                                     "pred_name2(?v - my_type, ?o - my_type2) - return_type", ontology.types);

  cp::Fact("pred_name(toto)", ontology, {}, {});
  cp::Fact("pred_name(sub_toto)", ontology, {}, {});

  try
  {
    cp::Fact("pred_that_does_not_exist(titi)", ontology, {}, {});
    assert_true(false);
  }
  catch(const std::exception& e) {
    assert_eq<std::string>("\"pred_that_does_not_exist\" is not a predicate name. The exception was thrown while parsing fact: \"pred_that_does_not_exist(titi)\"", e.what());
  }
  try
  {
    cp::Fact("pred_name(unknown_value)", ontology, {}, {});
    assert_true(false);
  }
  catch(const std::exception& e) {
    assert_eq<std::string>("\"unknown_value\" is not an entity value. The exception was thrown while parsing fact: \"pred_name(unknown_value)\"", e.what());
  }
  try
  {
    cp::Fact("pred_name", ontology, {}, {});
    assert_true(false);
  }
  catch(const std::exception& e) {
    assert_eq<std::string>("The fact \"pred_name\" does not have the same number of parameters than the associated predicate \"pred_name(?v - my_type)\". The exception was thrown while parsing fact: \"pred_name\"", e.what());
  }
  try
  {
    cp::Fact("pred_name(titi)", ontology, {}, {});
    assert_true(false);
  }
  catch(const std::exception& e) {
    assert_eq<std::string>("\"titi - my_type2\" is not a \"my_type\" for predicate: \"pred_name(?v - my_type)\". The exception was thrown while parsing fact: \"pred_name(titi)\"", e.what());
  }
  try
  {
    cp::Fact("pred_name(toto)=val", ontology, {}, {});
    assert_true(false);
  }
  catch(const std::exception& e) {
    assert_eq<std::string>("\"val\" is not an entity value. The exception was thrown while parsing fact: \"pred_name(toto)=val\"", e.what());
  }

  cp::Fact("pred_name2(toto, titi)=res", ontology, {}, {});
  {
    std::vector<cp::Parameter> parameters(1, cp::Parameter::fromStr("?v - my_type2", ontology.types));
    cp::Fact("pred_name2(toto, ?v)=res", ontology, {}, parameters);
  }
  {
    std::vector<cp::Parameter> parameters(1, cp::Parameter::fromStr("?v - my_type2", ontology.types));
    parameters.push_back( cp::Parameter::fromStr("?r - return_type", ontology.types));
    cp::Fact("pred_name2(toto, ?v)=?r", ontology, {}, parameters);
  }

  try
  {
    cp::Fact("pred_name2(toto, titi)=unknown_val", ontology, {}, {});
    assert_true(false);
  }
  catch(const std::exception& e) {
    assert_eq<std::string>("\"unknown_val\" is not an entity value. The exception was thrown while parsing fact: \"pred_name2(toto, titi)=unknown_val\"", e.what());
  }


  // Without ontology
  assert_eq<std::string>("pred(, )", cp::Fact("pred(lol, mdr)", {}, {}, {}).predicate.toStr());
  assert_eq<std::string>("pred(, ) - ", cp::Fact("pred(lol, mdr)=dd", {}, {}, {}).predicate.toStr());
}


void _test_action_initialization()
{
  cp::WorldState worldState;
  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("my_type my_type2 return_type\n"
                                           "sub_my_type - my_type");
  ontology.constants = cp::SetOfEntities::fromStr("toto - my_type\n"
                                                  "sub_toto - sub_my_type\n"
                                                  "titi tutu - my_type2\n"
                                                  "res - return_type", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("pred_name(?v - my_type)\n"
                                                     "pred_name2(?v - my_type, ?o - my_type2) - return_type\n"
                                                     "pred_name3(?o - my_type2) - return_type", ontology.types);

  cp::SetOfEntities entities;

  cp::Action action(cp::Condition::fromStr("pred_name(toto)", ontology, entities, {}),
                    cp::WorldStateModification::fromStr("pred_name2(toto, titi)=res", ontology, entities, {}));
  action.throwIfNotValid(worldState);

  {
    std::vector<cp::Parameter> parameters(1, cp::Parameter::fromStr("?p - my_type", ontology.types));
    cp::Action action2(cp::Condition::fromStr("pred_name(?p)", ontology, entities, parameters),
                      cp::WorldStateModification::fromStr("pred_name2(toto, titi)=res", ontology, entities, parameters));
    action2.parameters = std::move(parameters);
    action2.throwIfNotValid(worldState);
  }

  {
    std::vector<cp::Parameter> parameters(1, cp::Parameter::fromStr("?p - sub_my_type", ontology.types));
    cp::Action action3(cp::Condition::fromStr("pred_name(?p)", ontology, entities, parameters),
                      cp::WorldStateModification::fromStr("pred_name2(toto, titi)=res", ontology, entities, parameters));
    action3.parameters = std::move(parameters);
    action3.throwIfNotValid(worldState);
  }

  try
  {
    cp::Action action4(cp::Condition::fromStr("pred_name(?p)", ontology, entities, {}),
                       cp::WorldStateModification::fromStr("pred_name2(toto, titi)=res", ontology, entities, {}));
    action4.throwIfNotValid(worldState);
    assert_true(false);
  }
  catch (const std::exception& e)
  {
    assert_eq<std::string>("The parameter \"?p\" is unknown", e.what());
  }

  {
    std::vector<cp::Parameter> parameters(1, cp::Parameter::fromStr("?p - my_type", ontology.types));
    try
    {
      cp::Action action5(cp::Condition::fromStr("pred_name(?p)", ontology, entities, parameters),
                         cp::WorldStateModification::fromStr("pred_name2(toto, titi)=?r", ontology, entities, parameters));
      action5.parameters = std::move(parameters);
      action5.throwIfNotValid(worldState);
      assert_true(false);
    }
    catch (const std::exception& e)
    {
      assert_eq<std::string>("The parameter \"?r\" is unknown", e.what());
    }
  }

  cp::Condition::fromStr("exists(?obj - my_type2, pred_name2(toto, ?obj)=res)", ontology, entities, {});
  cp::Condition::fromStr("exists(?obj - my_type2, =(pred_name3(?obj), pred_name3(tutu)))", ontology, entities, {});
  cp::Condition::fromStr("=(pred_name3(tutu), undefined)", ontology, entities, {});
  cp::Condition::fromStr("=(pred_name3(tutu), res)", ontology, entities, {});
  cp::WorldStateModification::fromStr("forAll(?obj - my_type2, pred_name2(toto, ?obj)=res, set(pred_name3(?obj), pred_name3(tutu)))", ontology, entities, {});
  cp::WorldStateModification::fromStr("assign(pred_name3(tutu), res)", ontology, entities, {});
  std::vector<cp::Parameter> returnParameter(1, cp::Parameter::fromStr("?r - return_type", ontology.types));
  cp::WorldStateModification::fromStr("assign(pred_name3(tutu), ?r)", ontology, entities, returnParameter);
}


void _test_checkConditionWithOntology()
{
  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity\n"
                                           "my_type my_type2 - entity");
  ontology.constants = cp::SetOfEntities::fromStr("toto - my_type\n"
                                                  "titi - my_type2", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("pred_name(?e - entity)", ontology.types);

  cp::WorldState worldState;
  cp::GoalStack goalStack;
  std::map<cp::SetOfInferencesId, cp::SetOfInferences> setOfInferences;
  worldState.addFact(cp::Fact::fromStr("pred_name(toto)", ontology, {}, {}), goalStack, setOfInferences, ontology, {}, {});
  assert_false(cp::Condition::fromStr("pred_name(titi)", ontology, {}, {})->isTrue(worldState));

  {
    std::vector<cp::Parameter> parameters(1, cp::Parameter::fromStr("?p - my_type", ontology.types));
    auto parametersMap = _toParameterMap(parameters);
    assert_true(cp::Condition::fromStr("pred_name(?p)", ontology, {}, parameters)->isTrue(worldState, {}, {}, &parametersMap));
  }

  {
    std::vector<cp::Parameter> parameters(1, cp::Parameter::fromStr("?p - my_type2", ontology.types));
    auto parametersMap = _toParameterMap(parameters);
    assert_false(cp::Condition::fromStr("pred_name(?p)", ontology, {}, parameters)->isTrue(worldState, {}, {}, &parametersMap));
  }
}


}




void test_ontology()
{
  _test_setOfTypes();
  _test_setOfTypes_fromStr();
  _test_predicateToStr();
  _test_setOfPredicates_fromStr();
  _test_setOfEntities_fromStr();
  _test_fact_initialization();
  _test_action_initialization();
  _test_checkConditionWithOntology();

  std::cout << "ontology is ok !!!!" << std::endl;
}
