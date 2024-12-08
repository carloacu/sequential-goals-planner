#include <gtest/gtest.h>
#include <orderedgoalsplanner/types/goalstack.hpp>
#include <orderedgoalsplanner/types/ontology.hpp>
#include <orderedgoalsplanner/types/setofcallbacks.hpp>
#include <orderedgoalsplanner/types/setofevents.hpp>
#include <orderedgoalsplanner/types/setofpredicates.hpp>
#include <orderedgoalsplanner/types/worldstate.hpp>

using namespace ogp;

namespace
{

void _modifyFactsFromPddl(ogp::WorldState& pWorldstate,
                          const std::string& pStr,
                          const ogp::Ontology& pOntology,
                          const SetOfEntities& pEntities)
{
  std::size_t pos = 0;
  GoalStack goalStack;
  const std::map<SetOfEventsId, SetOfEvents> setOfEvents;
  const SetOfCallbacks callbacks;
  pWorldstate.modifyFactsFromPddl(pStr, pos, goalStack, setOfEvents, callbacks,
                                  pOntology, pEntities, {});
}
}


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

  _modifyFactsFromPddl(worldstate, "(pred_a toto)\n(pred_b)", ontology, entities);
  EXPECT_EQ("(pred_a toto)\n(pred_b)", worldstate.factsMapping().toPddl(0, true));
  _modifyFactsFromPddl(worldstate, "(= (pred_d titi) 4)\n(pred_a ent_a)", ontology, entities);
  EXPECT_EQ("(pred_a ent_a)\n(pred_a toto)\n(pred_b)\n(= (pred_d titi) 4)", worldstate.factsMapping().toPddl(0, true));
  _modifyFactsFromPddl(worldstate, "(= (pred_d titi) undefined)\n(pred_a titi)\n(not (pred_a toto))", ontology, entities);
  EXPECT_EQ("(pred_a ent_a)\n(pred_a titi)\n(pred_b)", worldstate.factsMapping().toPddl(0, true));
  _modifyFactsFromPddl(worldstate, "(not (pred_a titi))", ontology, entities);
  EXPECT_EQ("(pred_a ent_a)\n(pred_b)", worldstate.factsMapping().toPddl(0, true));
  _modifyFactsFromPddl(worldstate, "(= (pred_e ent_b) toto)\n(pred_b)", ontology, entities);
  EXPECT_EQ("(pred_a ent_a)\n(pred_b)\n(= (pred_e ent_b) toto)", worldstate.factsMapping().toPddl(0, true));
  _modifyFactsFromPddl(worldstate, "(= (pred_e ent_b) undefined)", ontology, entities);
  EXPECT_EQ("(pred_a ent_a)\n(pred_b)", worldstate.factsMapping().toPddl(0, true));
  _modifyFactsFromPddl(worldstate, "(= (pred_e ent_b) undefined)", ontology, entities);
  EXPECT_EQ("(pred_a ent_a)\n(pred_b)", worldstate.factsMapping().toPddl(0, true));
}
