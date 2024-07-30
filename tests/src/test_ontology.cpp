#include "test_arithmeticevaluator.hpp"
#include <assert.h>
#include <iostream>
#include <contextualplanner/types/action.hpp>
#include <contextualplanner/types/fact.hpp>
#include <contextualplanner/types/ontology.hpp>
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

  cp::Fact("pred_name(toto)", ontology, {});
  cp::Fact("pred_name(sub_toto)", ontology, {});

  try
  {
    cp::Fact("pred_that_does_not_exist(titi)", ontology, {});
    assert_true(false);
  }
  catch(const std::exception& e) {
    assert_eq<std::string>("\"pred_that_does_not_exist\" is not a predicate name. The exception was thrown while parsing fact: \"pred_that_does_not_exist(titi)\"", e.what());
  }
  try
  {
    cp::Fact("pred_name(unknown_value)", ontology, {});
    assert_true(false);
  }
  catch(const std::exception& e) {
    assert_eq<std::string>("\"unknown_value\" is not an entity value. The exception was thrown while parsing fact: \"pred_name(unknown_value)\"", e.what());
  }
  try
  {
    cp::Fact("pred_name", ontology, {});
    assert_true(false);
  }
  catch(const std::exception& e) {
    assert_eq<std::string>("The fact \"pred_name\" does not have the same number of parameters than the associated predicate \"pred_name(?v - my_type)\". The exception was thrown while parsing fact: \"pred_name\"", e.what());
  }
  try
  {
    cp::Fact("pred_name(titi)", ontology, {});
    assert_true(false);
  }
  catch(const std::exception& e) {
    assert_eq<std::string>("\"titi - my_type2\" is not a \"my_type\" for predicate: \"pred_name(?v - my_type)\". The exception was thrown while parsing fact: \"pred_name(titi)\"", e.what());
  }
  try
  {
    cp::Fact("pred_name(toto)=val", ontology, {});
    assert_true(false);
  }
  catch(const std::exception& e) {
    assert_eq<std::string>("\"val\" fluent is not a entity value. The exception was thrown while parsing fact: \"pred_name(toto)=val\"", e.what());
  }

  cp::Fact("pred_name2(toto, titi)=res", ontology, {});
  cp::Fact("pred_name2(toto, ?v)=res", ontology, {});
  cp::Fact("pred_name2(toto, ?v)=?r", ontology, {});

  try
  {
    cp::Fact("pred_name2(toto, titi)=unknown_val", ontology, {});
    assert_true(false);
  }
  catch(const std::exception& e) {
    assert_eq<std::string>("\"unknown_val\" fluent is not a entity value. The exception was thrown while parsing fact: \"pred_name2(toto, titi)=unknown_val\"", e.what());
  }


  // Without ontology
  assert_eq<std::string>("pred(, )", cp::Fact("pred(lol, mdr)", {}, {}).predicate.toStr());
  assert_eq<std::string>("pred(, ) - ", cp::Fact("pred(lol, mdr)=dd", {}, {}).predicate.toStr());
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
                                                     "pred_name2(?v - my_type, ?o - my_type2) - return_type", ontology.types);

  cp::SetOfEntities entities;

  cp::Action action(cp::Condition::fromStr("pred_name(toto)", ontology, entities),
                    cp::WorldStateModification::fromStr("pred_name2(toto, titi)=res", ontology, entities));
  action.limitPredicateTypes(worldState);


  cp::Action action2(cp::Condition::fromStr("pred_name(?p)", ontology, entities),
                    cp::WorldStateModification::fromStr("pred_name2(toto, titi)=res", ontology, entities));
  action2.parameters.emplace_back(cp::Parameter::fromStr("?p - my_type", ontology.types));
  action2.limitPredicateTypes(worldState);

  cp::Action action3(cp::Condition::fromStr("pred_name(?p)", ontology, entities),
                    cp::WorldStateModification::fromStr("pred_name2(toto, titi)=res", ontology, entities));
  try
  {
    action3.limitPredicateTypes(worldState);
    assert_true(false);
  }
  catch (const std::exception& e)
  {
    assert_eq<std::string>("\"?p\" is missing in action parameters", e.what());
  }

  cp::Action action4(cp::Condition::fromStr("pred_name(?p)", ontology, entities),
                    cp::WorldStateModification::fromStr("pred_name2(toto, titi)=?r", ontology, entities));
  action4.parameters.emplace_back(cp::Parameter::fromStr("?p - my_type", ontology.types));
  try
  {
    action4.limitPredicateTypes(worldState);
    assert_true(false);
  }
  catch (const std::exception& e)
  {
    assert_eq<std::string>("\"?r\" fluent is missing in action parameters", e.what());
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

  std::cout << "ontology is ok !!!!" << std::endl;
}
