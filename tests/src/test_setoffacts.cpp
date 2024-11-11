#include <gtest/gtest.h>
#include <prioritizedgoalsplanner/types/fact.hpp>
#include <prioritizedgoalsplanner/types/setoffacts.hpp>
#include <prioritizedgoalsplanner/types/ontology.hpp>
#include <prioritizedgoalsplanner/util/alias.hpp>

using namespace pgp;


TEST(Tool, test_setOfFacts)
{
  pgp::Ontology ontology;
  ontology.types = pgp::SetOfTypes::fromPddl("entity\n"
                                           "my_type my_type2 - entity\n"
                                           "sub_my_type2 - my_type2");
  ontology.constants = pgp::SetOfEntities::fromPddl("toto toto2 - my_type\n"
                                                  "titi titi_const - my_type2", ontology.types);
  ontology.predicates = pgp::SetOfPredicates::fromStr("pred_name(?e - entity)\n"
                                                     "pred_name2(?e - entity)\n"
                                                     "pred_name3(?p1 - my_type, ?p2 - my_type2)\n"
                                                     "pred_name4(?p1 - my_type, ?p2 - my_type2)\n"
                                                     "pred_name5(?p1 - my_type) - my_type2",
                                                     ontology.types);

  auto entities = pgp::SetOfEntities::fromPddl("toto3 - my_type\n"
                                             "titi2 - my_type2\n"
                                             "titi3 - sub_my_type2", ontology.types);

  SetOfFacts factToFacts;

  auto fact1 = pgp::Fact::fromStr("pred_name(toto)", ontology, entities, {});
  factToFacts.add(fact1);

  {
    EXPECT_EQ("[pred_name(toto)]", factToFacts.find(fact1).toStr());
  }

  EXPECT_TRUE(factToFacts.erase(fact1));

  {
    EXPECT_EQ("[]", factToFacts.find(fact1).toStr());
  }


  // tests with pred_name5

  auto fact2 = pgp::Fact::fromStr("pred_name5(toto2)=titi", ontology, entities, {});
  factToFacts.add(fact2, false);
  EXPECT_FALSE(factToFacts.erase(fact2));
  {
    EXPECT_EQ("[]", factToFacts.find(fact1).toStr());
  }

  auto fact3 = pgp::Fact::fromStr("pred_name5(toto)=titi", ontology, entities, {});
  factToFacts.add(fact3);

  auto fact4 = pgp::Fact::fromStr("pred_name5(toto)=titi_const", ontology, entities, {});
  factToFacts.add(fact4);

  {
    auto factWithParam = pgp::Fact::fromStr("pred_name5(toto)=titi", ontology, entities, {});
    EXPECT_EQ("[pred_name5(toto)=titi]", factToFacts.find(factWithParam).toStr());
    EXPECT_EQ("[pred_name5(toto)=titi, pred_name5(toto)=titi_const]", factToFacts.find(factWithParam, true).toStr());
  }

  {
    std::vector<pgp::Parameter> parameters(1, pgp::Parameter::fromStr("?p1 - my_type", ontology.types));
    auto factWithParam = pgp::Fact::fromStr("pred_name5(?p1)=titi_const", ontology, entities, parameters);
    EXPECT_EQ("[pred_name5(toto)=titi_const]", factToFacts.find(factWithParam).toStr());
    EXPECT_EQ("[pred_name5(toto2)=titi, pred_name5(toto)=titi, pred_name5(toto)=titi_const]", factToFacts.find(factWithParam, true).toStr());
  }

  auto factCopied = fact3;
  factToFacts.erase(factCopied);

  {
    auto factWithParam = pgp::Fact::fromStr("pred_name5(toto)=titi", ontology, entities, {});
    EXPECT_EQ("[]", factToFacts.find(factWithParam).toStr());
    EXPECT_EQ("[pred_name5(toto)=titi_const]", factToFacts.find(factWithParam, true).toStr());
  }

  auto fact4WithAnyValue = pgp::Fact::fromStr("pred_name5(toto)=*", ontology, entities, {});
  factToFacts.erase(fact4WithAnyValue);

  {
    auto factWithParam = pgp::Fact::fromStr("pred_name5(toto)=titi", ontology, entities, {});
    EXPECT_EQ("[]", factToFacts.find(factWithParam).toStr());
    EXPECT_EQ("[]", factToFacts.find(factWithParam, true).toStr());
  }
}
