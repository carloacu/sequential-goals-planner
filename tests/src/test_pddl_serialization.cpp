#include "test_pddl_serialization.hpp"
#include <assert.h>
#include <iostream>
#include <contextualplanner/types/ontology.hpp>
#include <contextualplanner/types/predicate.hpp>
#include <contextualplanner/types/setofentities.hpp>
#include <contextualplanner/types/setofpredicates.hpp>


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


}




void test_pddlSerialization()
{
  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("type1 type2 - entity");
  ontology.constants = cp::SetOfEntities::fromStr("toto - type1\n"
                                                  "titi - type2", ontology.types);

  cp::Predicate pred("(pred_a ?e - entity)", true, ontology.types);
  assert_eq<std::string>("pred_a(?e - entity)", pred.toStr());

  std::size_t pos = 0;
  ontology.predicates = cp::SetOfPredicates::fromPddl("(pred_a ?e - entity)\n"
                                                      "pred_b", pos, ontology.types);
  assert_eq<std::string>("pred_a(?e - entity)\n"
                         "pred_b()", ontology.predicates.toStr());

  std::cout << "PDDL serialization is ok !!!!" << std::endl;
}
