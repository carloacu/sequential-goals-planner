#include <gtest/gtest.h>
#include <orderedgoalsplanner/types/ontology.hpp>
#include <orderedgoalsplanner/types/setofpredicates.hpp>
#include <orderedgoalsplanner/types/worldstate.hpp>

using namespace ogp;


TEST(Tool, test_wordstate)
{
  ogp::WorldState worldstate;

  ogp::Ontology ontology;
  ontology.types = ogp::SetOfTypes::fromPddl("type1 type2 - entity");
  ontology.constants = ogp::SetOfEntities::fromPddl("ent_a ent_b - entity", ontology.types);

  {
    std::size_t pos = 0;
    ontology.predicates = ogp::SetOfPredicates::fromPddl("(pred_a ?e - entity)\n"
                                                         "pred_b\n"
                                                         "(pred_c ?t1 - type1)\n"
                                                         "(pred_d ?t2 - type2) - number\n"
                                                         "(pred_e ?e - entity) - type1", pos, ontology.types);
  }

  auto entities = ogp::SetOfEntities::fromPddl("toto - type1\n"
                                               "titi - type2", ontology.types);

  {
    std::size_t pos = 0;
    worldstate.modifyFactsFromPddl("(pred_a toto)\n(pred_b)", pos, ontology, entities);
  }
  EXPECT_EQ("(pred_a toto)\n(pred_b)", worldstate.factsMapping().toPddl(0, true));
  {
    std::size_t pos = 0;
    worldstate.modifyFactsFromPddl("(= (pred_d titi) 4)\n(pred_a ent_a)", pos, ontology, entities);
  }
  EXPECT_EQ("(pred_a ent_a)\n(pred_a toto)\n(pred_b)\n(= (pred_d titi) 4)", worldstate.factsMapping().toPddl(0, true));
  {
    std::size_t pos = 0;
    worldstate.modifyFactsFromPddl("(= (pred_d titi) undefined)\n(pred_a titi)\n(not (pred_a toto))", pos, ontology, entities);
  }
  EXPECT_EQ("(pred_a ent_a)\n(pred_a titi)\n(pred_b)", worldstate.factsMapping().toPddl(0, true));
  {
    std::size_t pos = 0;
    worldstate.modifyFactsFromPddl("(not (pred_a titi))", pos, ontology, entities);
  }
  EXPECT_EQ("(pred_a ent_a)\n(pred_b)", worldstate.factsMapping().toPddl(0, true));
  {
    std::size_t pos = 0;
    worldstate.modifyFactsFromPddl("(= (pred_e ent_b) toto)\n(pred_b)", pos, ontology, entities);
  }
  EXPECT_EQ("(pred_a ent_a)\n(pred_b)\n(= (pred_e ent_b) toto)", worldstate.factsMapping().toPddl(0, true));
  {
    std::size_t pos = 0;
    worldstate.modifyFactsFromPddl("(= (pred_e ent_b) undefined)", pos, ontology, entities);
  }
  EXPECT_EQ("(pred_a ent_a)\n(pred_b)", worldstate.factsMapping().toPddl(0, true));
  {
    std::size_t pos = 0;
    worldstate.modifyFactsFromPddl("(= (pred_e ent_b) undefined)", pos, ontology, entities);
  }
  EXPECT_EQ("(pred_a ent_a)\n(pred_b)", worldstate.factsMapping().toPddl(0, true));
}
