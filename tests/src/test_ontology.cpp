#include <gtest/gtest.h>
#include <prioritizedgoalsplanner/types/action.hpp>
#include <prioritizedgoalsplanner/types/fact.hpp>
#include <prioritizedgoalsplanner/types/goalstack.hpp>
#include <prioritizedgoalsplanner/types/ontology.hpp>
#include <prioritizedgoalsplanner/types/setofevents.hpp>
#include <prioritizedgoalsplanner/types/worldstate.hpp>
#include <prioritizedgoalsplanner/util/serializer/deserializefrompddl.hpp>

namespace
{
std::map<pgp::Parameter, std::set<pgp::Entity>> _toParameterMap(const std::vector<pgp::Parameter>& pParameters)
{
  std::map<pgp::Parameter, std::set<pgp::Entity>> res;
  for (auto& currParam : pParameters)
    res[currParam];
  return res;
}

void _test_setOfTypes()
{
  pgp::SetOfTypes setOfTypes;
  EXPECT_EQ("", setOfTypes.toStr());
  setOfTypes.addType("object");
  setOfTypes.addType("voiture", "object");
  setOfTypes.addType("maison", "object");
  setOfTypes.addType("citroen", "voiture");
  setOfTypes.addType("ferrari", "voiture");
  setOfTypes.addType("peugeot", "voiture");
  setOfTypes.addType("c3", "citroen");
  setOfTypes.addType("location");

  EXPECT_EQ("voiture maison - object\n"
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
  auto setOfTypes = pgp::SetOfTypes::fromPddl(typesStr + " ");
  EXPECT_EQ(typesStr, setOfTypes.toStr());
  EXPECT_EQ("voiture", setOfTypes.nameToType("citroen")->parent->name);
}


void _test_predicateToStr()
{
  auto setOfTypes = pgp::SetOfTypes::fromPddl("my_type my_type2 return_type");
  EXPECT_EQ("pred_name(?v - my_type)", pgp::Predicate("pred_name(?v - my_type)", false, setOfTypes).toStr());
  EXPECT_EQ("pred_name(?v - my_type, ?o - my_type2)", pgp::Predicate("pred_name(?v - my_type, ?o - my_type2)", false, setOfTypes).toStr());
  EXPECT_EQ("pred_name(?v - my_type, ?o - my_type2) - return_type", pgp::Predicate("pred_name(?v - my_type, ?o - my_type2) - return_type", false, setOfTypes).toStr());

  auto otherSetOfTypes = pgp::SetOfTypes::fromPddl("a b");
  try {
    pgp::Predicate("pred_name(?v - my_type)", false, otherSetOfTypes);
    EXPECT_TRUE(false);
  } catch (const std::exception& e) {
    EXPECT_EQ("\"my_type\" is not a valid type name", std::string(e.what()));
  }
  try {
    pgp::Predicate("pred_name(?v - a, ?o - b) - return_type", false, otherSetOfTypes);
    EXPECT_TRUE(false);
  } catch (const std::exception& e) {
    EXPECT_EQ("\"return_type\" is not a valid type name", std::string(e.what()));
  }
}


void _test_setOfPredicates_fromStr()
{
  auto setOfTypes = pgp::SetOfTypes::fromPddl("my_type my_type2 return_type");
  std::string predicatesStr = "pred_name(?v - my_type)\n"
                              "pred_name2(?v - my_type, ?o - my_type2)";
  auto setOfPredicates = pgp::SetOfPredicates::fromStr(predicatesStr, setOfTypes);
  EXPECT_EQ(predicatesStr, setOfPredicates.toStr());
}


void _test_setOfEntities_fromStr()
{
  auto setOfTypes = pgp::SetOfTypes::fromPddl("my_type my_type2 return_type");
  std::string entitiesStr = "toto - my_type\n"
                            "titi tutu - my_type2";
  auto setOfEntities = pgp::SetOfEntities::fromPddl(entitiesStr, setOfTypes);
  EXPECT_EQ(entitiesStr, setOfEntities.toStr());
  EXPECT_EQ("my_type2", setOfEntities.valueToEntity("titi")->type->name);
}

void _test_fact_initialization()
{
  pgp::Ontology ontology;
  ontology.types = pgp::SetOfTypes::fromPddl("my_type my_type2 return_type\n"
                                           "sub_my_type - my_type");
  ontology.constants = pgp::SetOfEntities::fromPddl("toto - my_type\n"
                                                  "sub_toto - sub_my_type\n"
                                                  "titi tutu - my_type2\n"
                                                  "res - return_type", ontology.types);
  ontology.predicates = pgp::SetOfPredicates::fromStr("pred_name(?v - my_type)\n"
                                                     "pred_name2(?v - my_type, ?o - my_type2) - return_type", ontology.types);

  pgp::Fact("pred_name(toto)", false, ontology, {}, {});
  pgp::Fact("pred_name(sub_toto)", false, ontology, {}, {});

  try
  {
    pgp::Fact("pred_that_does_not_exist(titi)", false, ontology, {}, {});
    EXPECT_TRUE(false);
  }
  catch(const std::exception& e) {
    EXPECT_EQ("\"pred_that_does_not_exist\" is not a predicate name. The exception was thrown while parsing fact: \"pred_that_does_not_exist(titi)\"", std::string(e.what()));
  }
  try
  {
    pgp::Fact("pred_name(unknown_value)", false, ontology, {}, {});
    EXPECT_TRUE(false);
  }
  catch(const std::exception& e) {
    EXPECT_EQ("\"unknown_value\" is not an entity value. The exception was thrown while parsing fact: \"pred_name(unknown_value)\"", std::string(e.what()));
  }
  try
  {
    pgp::Fact("pred_name", false, ontology, {}, {});
    EXPECT_TRUE(false);
  }
  catch(const std::exception& e) {
    EXPECT_EQ("The fact \"pred_name\" does not have the same number of parameters than the associated predicate \"pred_name(?v - my_type)\". The exception was thrown while parsing fact: \"pred_name\"", std::string(e.what()));
  }
  try
  {
    pgp::Fact("pred_name(titi)", false, ontology, {}, {});
    EXPECT_TRUE(false);
  }
  catch(const std::exception& e) {
    EXPECT_EQ("\"titi - my_type2\" is not a \"my_type\" for predicate: \"pred_name(?v - my_type)\". The exception was thrown while parsing fact: \"pred_name(titi)\"", std::string(e.what()));
  }
  try
  {
    pgp::Fact("pred_name(toto)=val", false, ontology, {}, {});
    EXPECT_TRUE(false);
  }
  catch(const std::exception& e) {
    EXPECT_EQ("\"val\" is not an entity value. The exception was thrown while parsing fact: \"pred_name(toto)=val\"", std::string(e.what()));
  }

  pgp::Fact("pred_name2(toto, titi)=res", false, ontology, {}, {});
  {
    std::vector<pgp::Parameter> parameters(1, pgp::Parameter::fromStr("?v - my_type2", ontology.types));
    pgp::Fact("pred_name2(toto, ?v)=res", false, ontology, {}, parameters);
  }
  {
    std::vector<pgp::Parameter> parameters(1, pgp::Parameter::fromStr("?v - my_type2", ontology.types));
    parameters.push_back( pgp::Parameter::fromStr("?r - return_type", ontology.types));
    pgp::Fact("pred_name2(toto, ?v)=?r", false, ontology, {}, parameters);
  }

  try
  {
    pgp::Fact("pred_name2(toto, titi)=unknown_val", false, ontology, {}, {});
    EXPECT_TRUE(false);
  }
  catch(const std::exception& e) {
    EXPECT_EQ("\"unknown_val\" is not an entity value. The exception was thrown while parsing fact: \"pred_name2(toto, titi)=unknown_val\"", std::string(e.what()));
  }
}


void _test_action_initialization()
{
  pgp::SetOfFacts setOfFacts;
  pgp::Ontology ontology;
  ontology.types = pgp::SetOfTypes::fromPddl("my_type my_type2 return_type\n"
                                           "sub_my_type - my_type");
  ontology.constants = pgp::SetOfEntities::fromPddl("toto - my_type\n"
                                                  "sub_toto - sub_my_type\n"
                                                  "titi tutu - my_type2\n"
                                                  "res - return_type", ontology.types);
  ontology.predicates = pgp::SetOfPredicates::fromStr("pred_name(?v - my_type)\n"
                                                     "pred_name2(?v - my_type, ?o - my_type2) - return_type\n"
                                                     "pred_name3(?o - my_type2) - return_type", ontology.types);

  pgp::SetOfEntities entities;

  pgp::Action action(pgp::strToCondition("pred_name(toto)", ontology, entities, {}),
                    pgp::strToWsModification("pred_name2(toto, titi)=res", ontology, entities, {}));
  action.throwIfNotValid(setOfFacts);

  {
    std::vector<pgp::Parameter> parameters(1, pgp::Parameter::fromStr("?p - my_type", ontology.types));
    pgp::Action action2(pgp::strToCondition("pred_name(?p)", ontology, entities, parameters),
                      pgp::strToWsModification("pred_name2(toto, titi)=res", ontology, entities, parameters));
    action2.parameters = std::move(parameters);
    action2.throwIfNotValid(setOfFacts);
  }

  {
    std::vector<pgp::Parameter> parameters(1, pgp::Parameter::fromStr("?p - sub_my_type", ontology.types));
    pgp::Action action3(pgp::strToCondition("pred_name(?p)", ontology, entities, parameters),
                      pgp::strToWsModification("pred_name2(toto, titi)=res", ontology, entities, parameters));
    action3.parameters = std::move(parameters);
    action3.throwIfNotValid(setOfFacts);
  }

  try
  {
    pgp::Action action4(pgp::strToCondition("pred_name(?p)", ontology, entities, {}),
                       pgp::strToWsModification("pred_name2(toto, titi)=res", ontology, entities, {}));
    action4.throwIfNotValid(setOfFacts);
    EXPECT_TRUE(false);
  }
  catch (const std::exception& e)
  {
    EXPECT_EQ("The parameter \"?p\" is unknown", std::string(e.what()));
  }

  {
    std::vector<pgp::Parameter> parameters(1, pgp::Parameter::fromStr("?p - my_type", ontology.types));
    try
    {
      pgp::Action action5(pgp::strToCondition("pred_name(?p)", ontology, entities, parameters),
                         pgp::strToWsModification("pred_name2(toto, titi)=?r", ontology, entities, parameters));
      action5.parameters = std::move(parameters);
      action5.throwIfNotValid(setOfFacts);
      EXPECT_TRUE(false);
    }
    catch (const std::exception& e)
    {
      EXPECT_EQ("The parameter \"?r\" is unknown", std::string(e.what()));
    }
  }

  pgp::strToCondition("exists(?obj - my_type2, pred_name2(toto, ?obj)=res)", ontology, entities, {});
  pgp::strToCondition("exists(?obj - my_type2, =(pred_name3(?obj), pred_name3(tutu)))", ontology, entities, {});
  pgp::strToCondition("=(pred_name3(tutu), undefined)", ontology, entities, {});
  pgp::strToCondition("=(pred_name3(tutu), res)", ontology, entities, {});
  pgp::strToWsModification("forall(?obj - my_type2, pred_name2(toto, ?obj)=res, set(pred_name3(?obj), pred_name3(tutu)))", ontology, entities, {});
  pgp::strToWsModification("assign(pred_name3(tutu), res)", ontology, entities, {});
  std::vector<pgp::Parameter> returnParameter(1, pgp::Parameter::fromStr("?r - return_type", ontology.types));
  pgp::strToWsModification("assign(pred_name3(tutu), ?r)", ontology, entities, returnParameter);
}


void _test_checkConditionWithOntology()
{
  pgp::Ontology ontology;
  ontology.types = pgp::SetOfTypes::fromPddl("entity\n"
                                           "my_type my_type2 - entity");
  ontology.constants = pgp::SetOfEntities::fromPddl("toto - my_type\n"
                                                  "titi - my_type2", ontology.types);
  ontology.predicates = pgp::SetOfPredicates::fromStr("pred_name(?e - entity)", ontology.types);

  pgp::WorldState worldState;
  pgp::GoalStack goalStack;
  std::map<pgp::SetOfEventsId, pgp::SetOfEvents> setOfEvents;
  worldState.addFact(pgp::Fact::fromStr("pred_name(toto)", ontology, {}, {}), goalStack, setOfEvents, ontology, {}, {});
  EXPECT_FALSE(pgp::strToCondition("pred_name(titi)", ontology, {}, {})->isTrue(worldState));

  {
    std::vector<pgp::Parameter> parameters(1, pgp::Parameter::fromStr("?p - my_type", ontology.types));
    auto parametersMap = _toParameterMap(parameters);
    EXPECT_TRUE(pgp::strToCondition("pred_name(?p)", ontology, {}, parameters)->isTrue(worldState, {}, {}, &parametersMap));
  }

  {
    std::vector<pgp::Parameter> parameters(1, pgp::Parameter::fromStr("?p - my_type2", ontology.types));
    auto parametersMap = _toParameterMap(parameters);
    EXPECT_FALSE(pgp::strToCondition("pred_name(?p)", ontology, {}, parameters)->isTrue(worldState, {}, {}, &parametersMap));
  }
}


}



TEST(Tool, test_ontology)
{
  _test_setOfTypes();
  _test_setOfTypes_fromStr();
  _test_predicateToStr();
  _test_setOfPredicates_fromStr();
  _test_setOfEntities_fromStr();
  _test_fact_initialization();
  _test_action_initialization();
  _test_checkConditionWithOntology();
}
