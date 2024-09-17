#include "test_setoffacts.hpp"
#include <assert.h>
#include <contextualplanner/types/fact.hpp>
#include <contextualplanner/types/setoffacts.hpp>
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


void test_setOfFacts()
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
                                                     "pred_name5(?p1 - my_type) - my_type2",
                                                     ontology.types);

  auto entities = cp::SetOfEntities::fromStr("toto3 - my_type\n"
                                             "titi2 - my_type2\n"
                                             "titi3 - sub_my_type2", ontology.types);

  SetOfFact factToFacts;

  auto fact1 = cp::Fact::fromStr("pred_name(toto)", ontology, entities, {});
  factToFacts.add(fact1);

  {
    assert_eq<std::string>("[pred_name(toto)]", factToFacts.find(fact1).toStr());
  }

  factToFacts.erase(fact1);

  {
    assert_eq<std::string>("[]", factToFacts.find(fact1).toStr());
  }


  // tests with pred_name5

  auto fact2 = cp::Fact::fromStr("pred_name5(toto2)=titi", ontology, entities, {});
  factToFacts.add(fact2);

  auto fact3 = cp::Fact::fromStr("pred_name5(toto)=titi", ontology, entities, {});
  factToFacts.add(fact3);

  auto fact4 = cp::Fact::fromStr("pred_name5(toto)=titi_const", ontology, entities, {});
  factToFacts.add(fact4);

  {
    auto factWithParam = cp::Fact::fromStr("pred_name5(toto)=titi", ontology, entities, {});
    assert_eq<std::string>("[pred_name5(toto)=titi]", factToFacts.find(factWithParam).toStr());
    assert_eq<std::string>("[pred_name5(toto)=titi, pred_name5(toto)=titi_const]", factToFacts.find(factWithParam, true).toStr());
  }

  {
    std::vector<cp::Parameter> parameters(1, cp::Parameter::fromStr("?p1 - my_type", ontology.types));
    auto factWithParam = cp::Fact::fromStr("pred_name5(?p1)=titi_const", ontology, entities, parameters);
    assert_eq<std::string>("[pred_name5(toto)=titi_const]", factToFacts.find(factWithParam).toStr());
    assert_eq<std::string>("[pred_name5(toto2)=titi, pred_name5(toto)=titi, pred_name5(toto)=titi_const]", factToFacts.find(factWithParam, true).toStr());
  }

  auto factCopied = fact3;
  factToFacts.erase(factCopied);

  {
    auto factWithParam = cp::Fact::fromStr("pred_name5(toto)=titi", ontology, entities, {});
    assert_eq<std::string>("[]", factToFacts.find(factWithParam).toStr());
    assert_eq<std::string>("[pred_name5(toto)=titi_const]", factToFacts.find(factWithParam, true).toStr());
  }

  auto fact4WithAnyValue = cp::Fact::fromStr("pred_name5(toto)=*", ontology, entities, {});
  factToFacts.erase(fact4WithAnyValue);

  {
    auto factWithParam = cp::Fact::fromStr("pred_name5(toto)=titi", ontology, entities, {});
    assert_eq<std::string>("[]", factToFacts.find(factWithParam).toStr());
    assert_eq<std::string>("[]", factToFacts.find(factWithParam, true).toStr());
  }
}
